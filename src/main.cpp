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
#include "ClockUtils.h"


using namespace std;
using namespace Xerxes;


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
static volatile bool core1idle = true;


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
    userInitGpio();
    userInitQueue();
    userInitFlash();

    // check if user switch is on, if so, use usb uart
    bool useUsb = gpio_get(USR_SW_PIN);

    if(useUsb)
    {
        // init usb uart
        stdio_usb_init();
        // wait for usb to be ready
        sleep_ms(2000);
        // cout labels of all values
        cout << "meanPv0;meanPv1;meanPv2;meanPv3;minPv0;minPv1;minPv2;minPv3;maxPv0;maxPv1;maxPv2;maxPv3;stdDevPv0;stdDevPv1;stdDevPv2;stdDevPv3;timestamp;netCycleTime" << endl;
        // cout separator for next line, char  # for the amount of previous characters
        cout << "###############################################################################################################################################################################" << endl;
    }
    else
    {
        // init uart over RS485
        userInitUart();
    }

    // if user button is pressed, load default values
    if(!gpio_get(USR_BTN_PIN)) userLoadDefaultValues();
        
    //determine reason for restart:
    if (watchdog_caused_reboot())
    {
        *error |= ERROR_WATCHDOG_TIMEOUT;
    }

    pSensor->init();

    // bind callbacks
    xs.bind(MSGID_PING,         unicast(    pingCallback));
    xs.bind(MSGID_WRITE,        unicast(    writeRegCallback));
    xs.bind(MSGID_READ,         unicast(    readRegCallback));
    xs.bind(MSGID_SYNC,         broadcast(  syncCallback));
    xs.bind(MSGID_SLEEP,        broadcast(  sleepCallback));
    xs.bind(MSGID_RESET,        broadcast(  softResetCallback));


    if(config->bits.freeRun)
    {
        multicore_launch_core1(core1Entry);
    }

    // enable watchdog for 100ms, pause on debug = true
    watchdog_enable(DEFAULT_WATCHDOG_DELAY, true);

    while(1)
    {    
        // update watchdog
        watchdog_update();

        if(useUsb)
        {
            // cout values of *meanPv0 to *meanPv3
            cout << *meanPv0 << ";" << *meanPv1 << ";" << *meanPv2 << ";" << *meanPv3 << ";";
            // cout values of *minPv0 to *minPv3
            cout << *minPv0 << ";" << *minPv1 << ";" << *minPv2 << ";" << *minPv3 << ";";
            // cout values of *maxPv0 to *maxPv3
            cout << *maxPv0 << ";" << *maxPv1 << ";" << *maxPv2 << ";" << *maxPv3 << ";";
            // cout values of *stdDevPv0 to *stdDevPv3
            cout << *stdDevPv0 << ";" << *stdDevPv1 << ";" << *stdDevPv2 << ";" << *stdDevPv3 << ";";
            // cout timestamp and net cycle time
            auto timestamp = time_us_64();
            cout << timestamp << ";" << *netCycleTimeUs << ";" << endl;
            

            // sleep in high speed mode for 1 second
            sleep_hp(1'000'000);
        }
        else
        {
            // try to send char over serial if present in FIFO buffer
            while(!queue_is_empty(&txFifo) && uart_is_writable(uart0)) // WARNING: Watchdog may reset here !!!
            {
                gpio_put(USR_LED_PIN, 1);
                uint8_t to_send, sent;
                auto removed = queue_try_remove(&txFifo, &to_send); 
                // TODO: remove all chars from queue and send them in one go

                // disable uart interrupt
                // irq_set_enabled(UART0_IRQ, false);

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

            // save power in release mode
            #ifdef NDEBUG
                if(core1idle)
                {
                    setClocksLP();
                    sleep_us(100);
                    setClocksHP();
                }
            #endif // NDEBUG
        }
    }
}


void core1Entry()
{
    uint64_t endOfCycle = 0;
    uint64_t cycleDuration = 0;
    int64_t sleepFor = 0;

    while(config->bits.freeRun)
    {
        auto startOfCycle = time_us_64();

        gpio_put(USR_LED_PIN, 1);
        syncOnce();          
        gpio_put(USR_LED_PIN, 0);

        // calculate how long it took to finish cycle
        endOfCycle = time_us_64();
        cycleDuration = endOfCycle - startOfCycle;
        *netCycleTimeUs = static_cast<uint32_t>(cycleDuration);
        sleepFor = *desiredCycleTimeUs - cycleDuration;
        
        // sleep for the remaining time
        if(sleepFor > 0)
        {
            core1idle = true;
            sleep_us(sleepFor);
            core1idle = false;
        }
        else
        {
            *error |= ERROR_SENSOR_OVERLOAD;
        }
    }
    
    core1idle = true;
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

        // if desired, calculate statistic values from ring buffer
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