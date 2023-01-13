#include <iostream>
#include <stdio.h>
#include <array>
#include <stdlib.h>
#include <bitset>
#include <cstdint>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/stdio/driver.h"
// #include "pico/stdio_uart.h"
#include "pico/sleep.h"

#include "Errors.h"
#include "Sensors/Honeywell/ABP.hpp"
#include "Slave.hpp"
#include "Protocol.hpp"
#include "Actions.hpp"


using namespace std;
using namespace Xerxes;


#ifdef TYPE_PRESSURE
Xerxes::Sensor *pSensor = new Xerxes::ABP();
#endif // TYPE_PRESSURE

Xerxes::RS485 xn(&txFifo, &rxFifo);
Xerxes::Protocol xp(&xn);
Xerxes::Slave xs(&xp, *devAddress, mainRegister);


static bool usrSwitchOn;
static volatile bool core1idle = true;


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
    xs.bind(MSGID_RESET_SOFT,   broadcast(  softResetCallback));
    xs.bind(MSGID_RESET_HARD,   unicast(    factoryResetCallback));


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
            

            // sleep in high speed mode for 1 second, watchdog friendly
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