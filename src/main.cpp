#include <iostream>
#include <stdio.h>
#include <array>
#include <stdlib.h>
#include <bitset>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "boards/pico.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"
#include "pico/stdio/driver.h"
#include "pico/stdio_uart.h"
#include "pico/sleep.h"
#include "hardware/rtc.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

#include <Serialization/Serialization.h>

#include "xerxes_rp2040.h"
#include "Errors.h"
#include "MessageId.h"
#include "DeviceIds.h"
#include "StatisticBuffer.hpp"
#include "Definitions.h"
#include "Sensors/Honeywell/ABP.hpp"
#include "Slave.hpp"
#include "Protocol.hpp"
#include "DirectionControl.hpp"


using namespace std;


#ifdef NDEBUG
#define DEBUG_MSG(str) do { } while ( false )
#else
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#endif


static const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static volatile bool awake = true;

/* Allocate memory for register */
volatile static uint8_t mainRegister[REGISTER_SIZE];

/* ### NON VOLATILE - PERMANENT VALUES ### */
static float* gainPv0       = (float *)(mainRegister + GAIN_PV0_OFFSET);
static float* gainPv1       = (float *)(mainRegister + GAIN_PV1_OFFSET);
static float* gainPv2       = (float *)(mainRegister + GAIN_PV2_OFFSET);
static float* gainPv3       = (float *)(mainRegister + GAIN_PV3_OFFSET);

static float* offsetPv0     = (float *)(mainRegister + OFFSET_PV0_OFFSET);
static float* offsetPv1     = (float *)(mainRegister + OFFSET_PV1_OFFSET);
static float* offsetPv2     = (float *)(mainRegister + OFFSET_PV2_OFFSET);
static float* offsetPv3     = (float *)(mainRegister + OFFSET_PV3_OFFSET);

static uint32_t *desiredCycleTimeUs  = (uint32_t *)(mainRegister + OFFSET_CYCLE_TIME);
static uint8_t *devAddress  = (uint8_t *)(mainRegister + OFFSET_ADDRESS);


/* ### VOLATILE - PROCESS VALUES ### */
static uint64_t* error      = (uint64_t *)(mainRegister + ERROR_OFFSET);
static uint64_t* status     = (uint64_t *)(mainRegister + STATUS_OFFSET);
static uint64_t* uid        = (uint64_t *)(mainRegister + UID_OFFSET);

static float* pv0           = (float *)(mainRegister + PV0_OFFSET);
static float* pv1           = (float *)(mainRegister + PV1_OFFSET);
static float* pv2           = (float *)(mainRegister + PV2_OFFSET);
static float* pv3           = (float *)(mainRegister + PV3_OFFSET);
std::array<float*, 4> processValues = {pv0, pv1, pv2, pv3};

static float* meanPv0       = (float *)(mainRegister + MEAN_PV0_OFFSET);
static float* meanPv1       = (float *)(mainRegister + MEAN_PV1_OFFSET);
static float* meanPv2       = (float *)(mainRegister + MEAN_PV2_OFFSET);
static float* meanPv3       = (float *)(mainRegister + MEAN_PV3_OFFSET);
std::array<float*, 4> meanValues = {meanPv0, meanPv1, meanPv2, meanPv3};

static float* stdDevPv0     = (float *)(mainRegister + STDEV_PV0_OFFSET);
static float* stdDevPv1     = (float *)(mainRegister + STDEV_PV1_OFFSET);
static float* stdDevPv2     = (float *)(mainRegister + STDEV_PV2_OFFSET);
static float* stdDevPv3     = (float *)(mainRegister + STDEV_PV3_OFFSET);
std::array<float*, 4> standardDeviations = {stdDevPv0, stdDevPv1, stdDevPv2, stdDevPv3};

static float* minPv0        = (float *)(mainRegister + MIN_PV0_OFFSET);
static float* minPv1        = (float *)(mainRegister + MIN_PV1_OFFSET);
static float* minPv2        = (float *)(mainRegister + MIN_PV2_OFFSET);
static float* minPv3        = (float *)(mainRegister + MIN_PV3_OFFSET);
std::array<float*, 4> minimumValues = {minPv0, minPv1, minPv2, minPv3};

static float* maxPv0        = (float *)(mainRegister + MAX_PV0_OFFSET);
static float* maxPv1        = (float *)(mainRegister + MAX_PV1_OFFSET);
static float* maxPv2        = (float *)(mainRegister + MAX_PV2_OFFSET);
static float* maxPv3        = (float *)(mainRegister + MAX_PV3_OFFSET);
std::array<float*, 4> maximumValues = {maxPv0, maxPv1, maxPv2, maxPv3};

// define ringbuffer (circular buffer) for each process value
Xerxes::StatisticBuffer<float> rbpv0(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv1(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv2(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv3(RING_BUFFER_LEN);

std::array<Xerxes::StatisticBuffer<float>,4> ringBuffers = {rbpv0, rbpv1, rbpv2, rbpv3};

Xerxes::Sensor *pSensor = new Xerxes::ABP(); 

static bool usrSwitchOn = false;

// queue for incoming and outgoing data
queue_t txFifo;
queue_t rxFifo;

Xerxes::RS485 xn(&txFifo, &rxFifo);
Xerxes::Protocol xp(&xn);
Xerxes::Slave xs(&xp, *devAddress, mainRegister);

void userInitUart();
void userInitGpio();
void userInitQueue();
void userInitFlash();
void userLoadDefaultValues();


void core1Entry();
void measurementLoop();
void uart_interrupt_handler();
void updateFlash();

void measureProcessValues(std::array<float*, 4> & processValues);


// This is the decorator function. It takes a function as an argument and returns
// a new function that adds additional behavior to the original function.
template <typename Func>
auto unicast(Func f) {
  // The returned function is a lambda function that takes the same arguments as the original
  // function and calls it with the given arguments.
  return [f](const Xerxes::Message &msg) {
    if(msg.dstAddr!= 0xff && *devAddress == msg.dstAddr)
    {
    // Call the original function with the given arguments.
        f(msg);
    }
    else
    {       
        // do nothing
    }
  };
}


void ping(const Xerxes::Message &msg)
{
    std::vector<uint8_t> payload {DEVID_PRESSURE_60MBAR, PROTOCOL_VERSION_MAJ, PROTOCOL_VERSION_MIN};
    xs.send(msg.srcAddr, MSGID_PING_REPLY, payload);
}


int main(void)
{   
	stdio_init_all();
    set_sys_clock_khz(133*KHZ, true);

    userInitUart();
    userInitGpio();
    userInitQueue();
    userInitFlash();
    updateFlash();
    userLoadDefaultValues();
    
    usrSwitchOn = gpio_get(USR_SW_PIN);

    // wait for button push
    /*while(gpio_get(USR_BTN_PIN))
    {
        gpio_put(USR_LED_PIN, !gpio_get(USR_LED_PIN));
        sleep_ms(100);
    }*/
    gpio_put(USR_LED_PIN, 0);

    // while in sleep run from lower powered XOSC (prlly 12MHz) 
    if(usrSwitchOn) sleep_run_from_xosc();

    //determine reason for restart:
    if (watchdog_caused_reboot())
    {
        *error |= ERROR_WATCHDOG_TIMEOUT;
    }

    cout << "UID: " << *uid << endl;
    // enable watchdog for 100ms, pause on debug = true
    watchdog_enable(100, true);
    cout << "Watchdog enabled for 100ms" << endl;

    //XerxesBusSetup(uart0_read, uart0_write, nop, nop, tx_done, uart0_is_rx_ready);
    //XerxesDeviceSetup(DEVID_IO_8DI_8DO, fetch_handler, nop);
    
    cout << "Errors: " << bitset<64>(*error) << endl;

    pSensor->init();

    xs.bind(MSGID_PING, unicast(ping));
    
    multicore_launch_core1(core1Entry);
    cout << "Core 1 launched. Communication ready!" << endl;
    

    while(1)
    {    
        // update watchdog
        watchdog_update();

        // try to read serial for incoming char
        /* handled in IRQ
        if(uart_is_readable(uart0))
        {
            gpio_put(USR_LED_PIN, 1);
            // read 1 char from serial
            uint8_t rcvd;
            uart_read_blocking(uart0, &rcvd, 1);

            //add it to the FIFO
            auto success = queue_try_add(&rxFifo, &rcvd);
            if(!success)
            {
                // set cpu overload flag
                *error |= ERROR_CPU_OVERLOAD;
+
            }
            gpio_put(USR_LED_PIN, 0);
        }
        */

        // try to send char over serial if present in FIFO buffer
        while(!queue_is_empty(&txFifo) && uart_is_writable(uart0))
        {
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
        }

        if(queue_is_full(&txFifo))
        {
            // rx fifo is full, set the cpu_overload error flag
            *error |= ERROR_UART_OVERLOAD;

        }

        xs.sync(3000);
    }
}


void core1Entry()
{
    uint64_t i=0;
    while(1)
    {
        if(awake)
        {
            auto startOfCycle = time_us_64();

            measurementLoop();

            // calculate how long it took to finish cycle
            auto endOfCycle = time_us_64();
            auto cycleDuration = endOfCycle - startOfCycle;
            int64_t sleepFor = *desiredCycleTimeUs - cycleDuration;

            if(i++ % 100 == 0)
            {
                DEBUG_MSG("Cycle duration: " << cycleDuration << "us.");
                DEBUG_MSG("Val: " << *meanPv0 << "Pa, stdev: " << *stdDevPv0);
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
        else
        {
            sleep_us(100);
        }
    }
}


void measurementLoop()
{       
    // measure process value
    pSensor->update();
    pSensor->read(processValues);

    // for each process value and ring buffer
    for(int i=0; i<4; i++)
    {
        float pv = *processValues[i];
        ringBuffers[i].insertOne(pv);
        ringBuffers[i].updateStatistics();
        // update min, max stdev etc...
        ringBuffers[i].getStatistics(
            minimumValues[i],
            maximumValues[i],
            meanValues[i],
            standardDeviations[i]
        );
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
    cout << "Baudrate:" << uart_init(uart0, 115200) << endl;
 
    // Set the GPIO pin mux to the UART - 16 is TX, 17 is RX
    gpio_set_function(RS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS_RX_PIN, GPIO_FUNC_UART);

    gpio_init(RS_EN_PIN);
    gpio_set_dir(RS_EN_PIN, GPIO_OUT);
    gpio_put(RS_EN_PIN, true);

    uart_set_fifo_enabled(uart0, 1);	
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
        char rcvd = uart_getc(uart0);

        auto success = queue_try_add(&rxFifo, &rcvd);
        if(!success)
        {
            // set cpu overload flag
            *error |= ERROR_CPU_OVERLOAD;
        }
    }

    irq_clear(UART0_IRQ);
    gpio_put(USR_LED_PIN, 0);
}


void userInitFlash()
{
    auto status = save_and_disable_interrupts();

    //read UID
    flash_get_unique_id((uint8_t *)uid);

    //erase flash, must be done in sector size
    // flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
}


void updateFlash()
{
    // disable interrupts first
    auto status = save_and_disable_interrupts();

    // write flash, must be done in page size
    // it takes approx 450us to write 128bytes of data
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t *)mainRegister, NON_VOLATILE_SIZE);

    restore_interrupts(status);
}


void userLoadDefaultValues()
{
    *gainPv0    = 1;
    *gainPv1    = 1;    
    *gainPv1    = 1;
    *gainPv1    = 1;

    *offsetPv0  = 0;
    *offsetPv0  = 0;
    *offsetPv0  = 0;
    *offsetPv0  = 0;

    *desiredCycleTimeUs = DEFAULT_CYCLE_TIME_US; 
}