#include <iostream>
#include <stdio.h>
#include <array>
#include <stdlib.h>
#include <bitset>
#include <cstdint>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "boards/pico.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdio/driver.h"
#include "pico/stdio_uart.h"
#include "pico/sleep.h"
#include "hardware/rtc.h"   
#include "hardware/watchdog.h"
#include "hardware/irq.h"

#include <Serialization/Serialization.h>

#include "xerxes_rp2040.h"
#include "Errors.h"
#include "Sensors/Honeywell/ABP.hpp"
#include "Slave.hpp"
#include "Protocol.hpp"
#include "Actions.hpp"


using namespace std;
using namespace Xerxes;


#ifdef NDEBUG
#define DEBUG_MSG(str) do { } while ( false )
#else
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#endif

#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )

// queue for incoming and outgoing data
queue_t txFifo;
queue_t rxFifo;


Xerxes::Sensor *pSensor = new Xerxes::ABP();
Xerxes::RS485 xn(&txFifo, &rxFifo);
Xerxes::Protocol xp(&xn);
Xerxes::Slave xs(&xp, *devAddress, mainRegister);


void userInitUsb()
{
    // either init usb or disable it and its clocks
    if(config->bits.disableUsb)
    {
        clock_configure(
            clk_usb,
            0,
            0,
            0,
            0
        );    
    }
    else
    {
        stdio_usb_init();
    }
}
 

void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
 
    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n\n", f_clk_rtc);
 
    // Can't measure clk_ref / xosc as it is the ref
}

static bool usrSwitchOn = false;


void userInitUart();
void userInitGpio();
void userInitQueue();
void userInitFlash();
void userLoadDefaultValues();
void uart_interrupt_handler();
void core1Entry();


int main(void)
{ 
    stdio_init_all();

    sleep_ms(3000);

    // sleep_lp(10);

    measure_freqs();

    userInitGpio();
    userInitUart();
    userInitQueue();
    userInitFlash();
    userInitUsb();

    // if user button is pressed, load default values
    if(!gpio_get(USR_BTN_PIN)) userLoadDefaultValues();
    
    usrSwitchOn = gpio_get(USR_SW_PIN);
    

    //determine reason for restart:
    if (watchdog_caused_reboot())
    {
        *error |= ERROR_WATCHDOG_TIMEOUT;
    }

    DEBUG_MSG("UID: " << *uid);
    // enable watchdog for 100ms, pause on debug = true
    watchdog_enable(100, true);
    DEBUG_MSG("Watchdog enabled for 100ms, Errors: " << bitset<64>(*error));


    pSensor->init();

    xs.bind(MSGID_PING,         unicast(    pingCallback));
    xs.bind(MSGID_WRITE,        unicast(    writeRegCallback));
    xs.bind(MSGID_READ,         unicast(    readRegCallback));
    xs.bind(MSGID_SYNC_ALL,     broadcast(  syncCallback));
    xs.bind(MSGID_SYNC_ONE,     unicast(    syncCallback));
    xs.bind(MSGID_SLEEP_ALL,    broadcast(  sleepCallback));
    xs.bind(MSGID_SLEEP_ONE,    unicast(    sleepCallback));
    xs.bind(MSGID_RESET,        broadcast(  softResetCallback));

    /* enable user interrupt 
    irq_set_exclusive_handler(26, user_interrupt_handler);
    irq_set_enabled(26, true); */

    if(config->bits.freeRun)
    {
        multicore_launch_core1(core1Entry);
    }

    DEBUG_MSG("Core 1 launched. Communication ready!");


    while(1)
    {    
        // update watchdog
        watchdog_update();

        // try to send char over serial if present in FIFO buffer
        while(!queue_is_empty(&txFifo) && uart_is_writable(uart0))
        {
            gpio_put(USR_LED_PIN, 1);
            uint8_t to_send, sent;
            queue_try_remove(&txFifo, &to_send);

            //write char to bus
            uart_write_blocking(uart0, &to_send, 1);
            // read back the character
            uart_read_blocking(uart0, &sent, 1);

            // check if sent character is the same as received = no collision on the bus
            if(to_send != sent)
            {
                *error |= ERROR_BUS_COLLISION;
            }
            gpio_put(USR_LED_PIN, 0);
        }

        if(queue_is_full(&txFifo))
        {
            // rx fifo is full, set the cpu_overload error flag
            *error |= ERROR_UART_OVERLOAD;

        }

        xs.sync(5000);
    }
}


void core1Entry()
{
    uint64_t i=0;
    while(config->bits.freeRun)
    {
        auto startOfCycle = time_us_64();

        measurementLoop();          

        // calculate how long it took to finish cycle
        auto endOfCycle = time_us_64();
        auto cycleDuration = endOfCycle - startOfCycle;
        int64_t sleepFor = *desiredCycleTimeUs - cycleDuration;

        if(i++ % 100 == 0)
        {
            gpio_put(USR_LED_PIN, 1);
            DEBUG_MSG("Cycle duration: " << cycleDuration << "us.");
            DEBUG_MSG("Val: " << *meanPv0 << "Pa, stddev: " << *stdDevPv0);
            sleep_us(100);
            gpio_put(USR_LED_PIN, 0);
        }

        // sleep for the remaining time
        if(sleepFor > 0)
        {
            sleep_us(sleepFor);
        }
        else
        {
            *error |= ERROR_SENSOR_OVERLOAD;
        }
    }
}



void userInitQueue()
{
    queue_init(&txFifo, 1, RX_TX_QUEUE_SIZE);
    queue_init(&rxFifo, 1, RX_TX_QUEUE_SIZE);
}


void userInitGpio()
{
    gpio_init(USR_SW_PIN);
	gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_init(USR_BTN_PIN);
    
    gpio_set_dir(USR_SW_PIN, GPIO_IN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_set_dir(USR_BTN_PIN, GPIO_IN);

    gpio_pull_up(USR_SW_PIN);
    gpio_pull_up(USR_BTN_PIN);

    gpio_put(RS_EN_PIN, true);
}


void userInitUart(void)
{
    // Initialise UART 0 on 115200baud
    DEBUG_MSG("Baudrate:" << uart_init(uart0, DEFAULT_BAUDRATE));
 
    // Set the GPIO pin mux to the UART - 16 is TX, 17 is RX
    gpio_set_function(RS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS_RX_PIN, GPIO_FUNC_UART);

    gpio_init(RS_EN_PIN);
    gpio_set_dir(RS_EN_PIN, GPIO_OUT);
    gpio_put(RS_EN_PIN, true);

    uart_set_fifo_enabled(uart0, true);	
    stdio_set_driver_enabled(&stdio_uart, false);

    irq_set_exclusive_handler(UART0_IRQ, uart_interrupt_handler);
    irq_set_enabled(UART0_IRQ, true);

    // enable uart interrupt for receiving
    uart_set_irq_enables(uart0, true, false);
}


void uart_interrupt_handler()
{
    gpio_put(USR_LED_PIN, 1);
    if(uart_is_readable(uart0))
    {
        unsigned char rcvd = uart_getc(uart0);
        auto success = queue_try_add(&rxFifo, &rcvd);

        if(!success)
        {
            // set cpu overload flag
            *error |= ERROR_CPU_OVERLOAD;
        }
    }
    gpio_put(USR_LED_PIN, 0);
    irq_clear(UART0_IRQ);
}


void userLoadDefaultValues()
{

    for(uint i=0; i<REGISTER_SIZE; i++)
    {
        mainRegister[i] = 0;
    }

    *gainPv0    = 1;
    *gainPv1    = 1;    
    *gainPv1    = 1;
    *gainPv1    = 1;

    *desiredCycleTimeUs = DEFAULT_CYCLE_TIME_US; 
    config->all = 0;
    *clockKhz = DEFAULT_CLOCK_KHZ;  // not implemented yet
    updateFlash();
}
