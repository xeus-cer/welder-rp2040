#include "xerxes_rp2040.h"
#include <iostream>
#include <stdio.h>
#include <array>
#include <stdlib.h>
#include <bitset>
#include <cstdint>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/stdio/driver.h"
#include "pico/sleep.h"
#include "hardware/xosc.h"

#include "Errors.h"
#include "Sensors/all.hpp"
#include "Slave.hpp"
#include "Protocol.hpp"
#include "Actions.hpp"

using namespace std;
using namespace Xerxes;

// static ABP sensor;           // pressure sensor 0-60mbar
// static SCL3300 sensor;       // 3 axis inclinometer
// static SCL3400 sensor;       // 3 axis inclinometer
// static AnalogInput sensor;   // 4 channel analog input
static AnalogInput sensor;  ///< 4 channel analog input


RS485 xn(&txFifo, &rxFifo);     ///< RS485 interface
Protocol xp(&xn);               ///< Xerxes protocol implementation
Slave xs(&xp, *devAddress, mainRegister);   ///< Xerxes slave implementation

static bool usrSwitchOn;                ///< user switch state
static volatile bool core1idle = true;  ///< core1 idle flag
volatile static bool useUsb = false;    ///< use usb uart flag

/**
 * @brief Core 1 entry point, runs in background
 */
void core1Entry();  

/**
 * @brief Poll sensor and update values
 */
void pollSensor();

// inline calculate transfer speed from DEFAULT_BAUDRATE of uart in bytes/s
constexpr uint32_t transferSpeed = (DEFAULT_BAUDRATE / 10); // 10 bits per byte (8 data bits + 1 start bit + 1 stop bit)

// inline calculate max transfer time in ms
constexpr uint32_t transferTime = (RX_TX_QUEUE_SIZE / transferSpeed) * 1000;


int main(void)
{
    // enable watchdog for 200ms, pause on debug = true
    watchdog_enable(DEFAULT_WATCHDOG_DELAY, true);
    
    // init system
    userInit();  // 374us
        
    // clear error register
    *error = 0;
    //determine reason for restart:
    if (watchdog_caused_reboot())
    {
        *error |= ERROR_MASK_WATCHDOG_TIMEOUT;
    }
    
    // check if user switch is on, if so, use usb uart
    useUsb = gpio_get(USR_SW_PIN);
    
    if(useUsb)
    {
        // init usb uart
        stdio_usb_init();
        // wait for usb to be ready
        sleep_hp(2'000'000);
        // print out error register
        cout << "error register: " << bitset<32>(*error) << endl;
        // cout labels of all values
        cout << "PV0;PV1;PV2;PV3;meanPv0;meanPv1;meanPv2;meanPv3;minPv0;minPv1;minPv2;minPv3;maxPv0;maxPv1;maxPv2;maxPv3;stdDevPv0;stdDevPv1;stdDevPv2;stdDevPv3;timestamp;netCycleTime" << endl;
        // set to free running mode
        config->bits.freeRun = 1;
        config->bits.calcStat = 1;
    }
    else
    {
        // init uart over RS485
        userInitUart();
    }
    
    // if user button is pressed, load default values a.k.a. FACTORY RESET
    if(!gpio_get(USR_BTN_PIN)) userLoadDefaultValues();
    
    // init sensor, depending on type of sensor selected 
    // sensor = Xerxes::ABP();
    // sensor = Xerxes::SCL3400();
    // sensor = Xerxes::SCL3300();
    // sensor = Xerxes::AnalogInput(2, 3);
    watchdog_update();
    sensor = Xerxes::AnalogInput(2, 3);  // 2 channels, 3 bit oversampling, approx 320us per update
    sensor.init();
    watchdog_update();

    // bind callbacks, 204us
    xs.bind(MSGID_PING,         unicast(    pingCallback));
    xs.bind(MSGID_WRITE,        unicast(    writeRegCallback));
    xs.bind(MSGID_READ,         unicast(    readRegCallback));
    xs.bind(MSGID_SYNC,         broadcast(  syncCallback));
    xs.bind(MSGID_SLEEP,        broadcast(  sleepCallback));
    xs.bind(MSGID_RESET_SOFT,   broadcast(  softResetCallback));
    xs.bind(MSGID_RESET_HARD,   unicast(    factoryResetCallback));

    // drain uart fifos, just in case there is something in there
    while(!queue_is_empty(&txFifo)) queue_remove_blocking(&txFifo, NULL);
    while(!queue_is_empty(&rxFifo)) queue_remove_blocking(&rxFifo, NULL);

    // start core1
    multicore_launch_core1(core1Entry);

    // main loop, runs forever, handles all communication in this loop
    while(1)
    {    
        // update watchdog
         watchdog_update();

        if(useUsb)
        {
            // cout values of *pv0 to *pv3
            cout << *pv0 << ";" << *pv1 << ";" << *pv2 << ";" << *pv3 << ";";
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
            // running on RS485, sync for incoming messages from master, timeout = 5ms
            xs.sync(5000);
            
            // send char if tx queue is not empty and uart is writable
            if(!queue_is_empty(&txFifo))
            {   
                uint txLen = queue_get_level(&txFifo);
                assert(txLen <= RX_TX_QUEUE_SIZE);

                uint8_t toSend[txLen];

                // drain queue
                for(uint i = 0; i < txLen; i++)
                {
                    queue_remove_blocking(&txFifo, &toSend[i]);
                }

                // write char to bus, this will clear the interrupt
                uart_write_blocking(uart0, toSend, txLen);
            }
        
            if(queue_is_full(&txFifo) || queue_is_full(&rxFifo))
            {
                // rx fifo is full, set the cpu_overload error flag
                *error |= ERROR_MASK_UART_OVERLOAD;
            }

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
    
    // let core0 lockout core1
    multicore_lockout_victim_init ();

    // core1 mainloop
    while(true)
    {
        // core is set to free run, start cycle
        auto startOfCycle = time_us_64();

        // turn on led for a short time to signal start of cycle
        gpio_put(USR_LED_PIN, 1);

        if(config->bits.freeRun)
        {
            pollSensor();   
        }

        // calculate how long it took to finish cycle
        endOfCycle = time_us_64();
        cycleDuration = endOfCycle - startOfCycle;
        *netCycleTimeUs = static_cast<uint32_t>(cycleDuration);
        sleepFor = *desiredCycleTimeUs - cycleDuration;

        // turn off led
        gpio_put(USR_LED_PIN, 0);
        
        // sleep for the remaining time
        if(sleepFor > 0)
        {
            core1idle = true;
            sleep_us(sleepFor);
            core1idle = false;
        }
        else
        {
            *error |= ERROR_MASK_SENSOR_OVERLOAD;
        }
        
    }
    
    core1idle = true;
}


void pollSensor()
{       
    // measure process value
    sensor.update();    
    // read process values from sensor
    sensor.read(processValues);

    // for each process value and ring buffer
    for(int i=0; i<4; i++)
    {
        // insert process value into ring buffer
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