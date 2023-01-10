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
#include "hardware/pwm.h"
#include "pico/stdio/driver.h"
// #include "pico/stdio_uart.h"
#include "pico/sleep.h"
#include "hardware/rtc.h"   
#include "hardware/watchdog.h"
#include "hardware/irq.h"

#include "xerxes_rp2040.h"
#include "Errors.h"
#include "Sensors/Honeywell/ABP.hpp"
#include "Slave.hpp"
#include "Protocol.hpp"
#include "Actions.hpp"
#include "ClockUtils.hpp"


using namespace std;
using namespace Xerxes;


#ifdef NDEBUG
// in release build
// #define DEBUG_MSG(str) do { } while ( false )
#else
// in debug
#endif

#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )


// queue for incoming and outgoing data
queue_t txFifo;
queue_t rxFifo;

#ifdef TYPE_PRESSURE
Xerxes::Sensor *pSensor = new Xerxes::ABP();
#endif // TYPE_PRESSURE

Xerxes::RS485 xn(&txFifo, &rxFifo);
Xerxes::Protocol xp(&xn);
Xerxes::Slave xs(&xp, *devAddress, mainRegister);


static bool usrSwitchOn;
static volatile bool busy = false;


void userInitUart();
void userInitGpio();
void userInitQueue();
void userInitFlash();
void userLoadDefaultValues();
void uart_interrupt_handler();
void core1Entry();
void syncOnce();


int main(void)
{ 
    userInitClocks();
    stdio_init_all();
    userInitGpio();
    stdio_usb_init();
    userInitUart();
    userInitQueue();
    userInitFlash();
    measure_freqs();

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
    watchdog_enable(DEFAULT_WATCHDOG_DELAY, true);
    DEBUG_MSG("Watchdog enabled for 100ms, Errors: " << bitset<64>(*error));


    pSensor->init();

    // bind callbacks
    xs.bind(MSGID_PING,         unicast(    pingCallback));
    xs.bind(MSGID_WRITE,        unicast(    writeRegCallback));
    xs.bind(MSGID_READ,         unicast(    readRegCallback));
    xs.bind(MSGID_SYNC,         broadcast(  syncCallback));
    xs.bind(MSGID_SLEEP,        broadcast(  sleepCallback));
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
        while(!queue_is_empty(&txFifo) && uart_is_writable(uart0)) // FIXME - Watchdog may reset here !!!
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

        /* Sync and return in less than 5ms */
        xs.sync(5000);

        #ifdef NDEBUG
        /* Save some power in Release by lowering the clock */
        if(!busy)
        {
            setClockSysLP();
            sleep_us(50);
            setClockSysDefault();
        }else{
            sleep_us(50);
        }
        #endif // NDEBUG

    }
}


void core1Entry()
{
    uint64_t i=0;
    while(config->bits.freeRun)
    {
        auto startOfCycle = time_us_64();
        syncOnce();          

        // calculate how long it took to finish cycle
        auto endOfCycle = time_us_64();
        auto cycleDuration = endOfCycle - startOfCycle;
        int64_t sleepFor = *desiredCycleTimeUs - cycleDuration;

        if(i++ % 100 == 0)
        {
            gpio_put(USR_LED_PIN, 1);
            DEBUG_MSG("Cycle duration: " << cycleDuration << "us.");
            DEBUG_MSG("Val: " << *meanPv0 << "Pa, stddev: " << *stdDevPv0);
            gpio_put(USR_LED_PIN, 0);
        }
        // sleep for the remaining time
        if(sleepFor > 0)
        {
            busy = false;   
            sleep_us(sleepFor);
            busy = true;
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
	gpio_init(USR_LED_PIN);
    gpio_init(USR_BTN_PIN);
    
    gpio_set_dir(USR_SW_PIN, GPIO_IN);
	gpio_set_dir(USR_LED_PIN, GPIO_OUT);
    gpio_set_dir(USR_BTN_PIN, GPIO_IN);

    gpio_pull_up(USR_SW_PIN);
    gpio_pull_up(USR_BTN_PIN);

    gpio_put(RS_EN_PIN, true);
}


void userInitUart(void)
{
    // Initialise UART 0 on 115200baud
    uart_init(uart0, DEFAULT_BAUDRATE);
 
    // Set the GPIO pin mux to the UART - 16 is TX, 17 is RX
    gpio_set_function(RS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS_RX_PIN, GPIO_FUNC_UART);

    gpio_init(RS_EN_PIN);
    gpio_set_dir(RS_EN_PIN, GPIO_OUT);
    gpio_put(RS_EN_PIN, true);

    uart_set_fifo_enabled(uart0, true);	
    // stdio_set_driver_enabled(&stdio_uart, false);

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
    updateFlash();
}


void syncOnce()
{       
    // measure process value
    pSensor->update();
    pSensor->read(processValues);

    // for each process value and ring buffer
    for(int i=0; i<4; i++)
    {
        float pv = *processValues[i];
        ringBuffers[i].insertOne(pv);
        if(config->bits.calcStat)
        {
            ringBuffers[i].updateStatistics();
            // update min, max stddev etc...
            ringBuffers[i].getStatistics(
                minimumValues[i],
                maximumValues[i],
                meanValues[i],
                standardDeviations[i]
            );
        }
    }

}