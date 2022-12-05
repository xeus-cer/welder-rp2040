#include <iostream>
#include <stdio.h>
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
#include <vector>

#include "xerxes_rp2040.h"
#include "errors.h"
#include "protocol.h"
#include "devids.h"

#include "xbus_485.hpp"
#include "xprotocol.hpp"

using namespace std;

#define RX_TX_QUEUE_SIZE 256 // bytes
// #undef PICO_FLASH_SIZE_BYTES
#define FLASH_TARGET_OFFSET 2 * 1024 * 1024 - (FLASH_SECTOR_SIZE)
static const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
#define REGISTER_SIZE 256
#define NON_VOLATILE_SIZE 128

volatile static uint8_t main_register[REGISTER_SIZE];

#define STATUS_OFFSET   NON_VOLATILE_SIZE   // 128
#define ERROR_OFFSET    STATUS_OFFSET + 8   // 136
#define UID_OFFSET      ERROR_OFFSET + 8    // 144

static uint64_t* error = (uint64_t *)(main_register + ERROR_OFFSET);
static uint64_t* status = (uint64_t *)(main_register + STATUS_OFFSET);
static uint64_t* uid = (uint64_t *)(main_register + UID_OFFSET);


static bool powersafe = false;
queue_t tx_fifo;
queue_t rx_fifo;

void user_init_uart();
void user_init_gpio();
void user_init_queue();
void user_init_flash();

bool tx_done();
void nop();
uint8_t uart0_read();
void uart0_write(uint8_t c);
bool uart0_is_rx_ready();
void fetch_handler(uint8_t c);
void core1_entry();
void uart_interrupt_handler();
void update_flash();

bool has_valid_xerxes_message(queue_t* qrx);


int main(void)
{   
    XBus_485()
    XProtocol xp()

	stdio_init_all();
    set_sys_clock_khz(133*KHZ, true);

    user_init_uart();
    user_init_gpio();
    user_init_queue();
    user_init_flash();
    update_flash();

    powersafe = gpio_get(USR_SW_PIN);
    while(gpio_get(USR_BTN_PIN));

    // while in sleep run from lower powered XOSC (prlly 12MHz) 
    if(powersafe) sleep_run_from_xosc();

    //XerxesBusSetup(uart0_read, uart0_write, nop, nop, tx_done, uart0_is_rx_ready);
    //XerxesDeviceSetup(DEVID_IO_8DI_8DO, fetch_handler, nop);
    
    
    multicore_launch_core1(core1_entry);

    for(auto i=0; i<128; i++)
    {
        printf("%X", flash_target_contents[i]);
    }
    cout << endl << "UID: " << *uid << endl;
    cout << "Core 1 launched. Communication ready!" << endl;


    while(1)
    {
        //if (powersafe) gpio_put(RS_EN_PIN, false);
        
        uint8_t rcvd;
        if(queue_try_remove(&rx_fifo, &rcvd))
        {
            printf("%c", rcvd);
            // stdio_flush();
        }
        
        tight_loop_contents();
    }
}


void user_init_queue()
{
    queue_init(&tx_fifo, 1, RX_TX_QUEUE_SIZE);
    queue_init(&rx_fifo, 1, RX_TX_QUEUE_SIZE);
}


void user_init_gpio()
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


void user_init_uart(void)
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

    // irq_set_exclusive_handler(UART0_IRQ, uart_interrupt_handler);
    // irq_set_enabled(UART0_IRQ, true);

    // enable uart interrupt
    // uart_set_irq_enables(uart0, true, false);
}


bool tx_done()
{
    return queue_is_empty(&tx_fifo);
}


void nop()
{
    __asm("nop");
}


uint8_t uart0_read()
{
    uint8_t rcvd;
    queue_remove_blocking(&rx_fifo, &rcvd);
    return rcvd;
}


void uart0_write(uint8_t c)
{
    queue_add_blocking(&tx_fifo, &c);
}


bool uart0_is_rx_ready()
{
    return !queue_is_empty(&rx_fifo);
}


void fetch_handler(uint8_t c)
{
    gpio_put(USR_LED_PIN, 1);
    sleep_ms(5);
    gpio_put(USR_LED_PIN, 0);
}


void core1_entry()
{
    while(1)
    {    
        // try to read serial for incoming char
        if(uart_is_readable(uart0))
        {
            gpio_put(USR_LED_PIN, 1);
            // read 1 char from serial
            uint8_t rcvd;
            uart_read_blocking(uart0, &rcvd, 1);

            //add it to the FIFO
            auto success = queue_try_add(&rx_fifo, &rcvd);
            if(!success)
            {
                // set cpu overload flag
                *error |= ERROR_CPU_OVERLOAD;

            }
            gpio_put(USR_LED_PIN, 0);
        }

        // try to send char over serial if present in FIFO buffer
        if(!queue_is_empty(&tx_fifo) && uart_is_writable(uart0))
        {
            uint8_t to_send, sent;
            queue_try_remove(&tx_fifo, &to_send);

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

        if(queue_is_full(&tx_fifo))
        {
            // rx fifo is full, set the cpu_overload error flag
            *error |= ERROR_UART_OVERLOAD;
        }
    }
}


void uart_interrupt_handler()
{
    gpio_put(USR_LED_PIN, 1);
    if(uart_is_readable(uart0))
    {
        char rcvd = uart_getc(uart0);
        queue_try_add(&rx_fifo, &rcvd);
    }

    irq_clear(UART0_IRQ);
    gpio_put(USR_LED_PIN, 0);
}


void user_init_flash()
{
    auto status = save_and_disable_interrupts();

    //read UID
    flash_get_unique_id((uint8_t *)uid);

    //erase flash, must be done in sector size
    // flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
}


void update_flash()
{
    // disable interrupts first
    auto status = save_and_disable_interrupts();

    // write flash, must be done in page size
    // it takes approx 450us to write 128bytes of data
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t *)main_register, NON_VOLATILE_SIZE);

    restore_interrupts(status);
}