#include <iostream>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "boards/pico.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "xerxes-protocol-cpp/include/protocol.h"
#include "xerxes-protocol-cpp/include/devids.h"
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

#include "xerxes_rp2040.h"


using namespace std;

#define RX_TX_QUEUE_SIZE 256 // bytes

static bool powersave = false;

queue_t tx_fifo;
queue_t rx_fifo;

uint8_t this_dev_addr;

void user_init_uart();
void user_init_gpio();
void user_init_queue();
bool tx_done();
void nop();
uint8_t uart0_read();
void uart0_write(uint8_t c);
bool uart0_is_rx_ready();
void fetch_handler(uint8_t c);
void toggler();
void mainloop(void);
void uart_interrupt_handler();


int main(void)
{

    this_dev_addr = 0x01;

	stdio_init_all();
    set_sys_clock_khz(133*KHZ, true);

    // while in sleep run from lower powered XOSC (prlly 12MHz) 
    if(powersave) sleep_run_from_xosc();

    user_init_uart();
    user_init_gpio();
    user_init_queue();
    powersave = gpio_get(USR_SW_PIN);

    XerxesBusSetup(uart0_read, uart0_write, nop, nop, tx_done, uart0_is_rx_ready);
    XerxesDeviceSetup(DEVID_IO_8DI_8DO, fetch_handler, nop);
    
    multicore_launch_core1(toggler);


    while(1)
    {
        gpio_put(RS_EN_PIN, true);
        uart_putc(uart0, '#');
        uart_tx_wait_blocking(uart0);
        
        if (powersave) gpio_put(RS_EN_PIN, false);
        cout << gpio_get(USR_SW_PIN);

        sleep_ms(1000);
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

    irq_set_exclusive_handler(UART0_IRQ, uart_interrupt_handler);
    irq_set_enabled(UART0_IRQ, true);
    // enable uart interrupt for TX needs data, disable for RX has data
    uart_set_irq_enables(uart0, true, false);
}


bool tx_done()
{
    uart_tx_wait_blocking(uart0);
    return 1;
}


void nop()
{
    __asm("nop");
}


uint8_t uart0_read()
{
    return uart_getc(uart0);
}


void uart0_write(uint8_t c)
{
    uart_putc_raw(uart0, c);
}


bool uart0_is_rx_ready()
{
    return uart_is_readable(uart0);
}


void fetch_handler(uint8_t c)
{
    gpio_put(USR_LED_PIN, 1);
    sleep_ms(5);
    gpio_put(USR_LED_PIN, 0);
}


void toggler()
{
    char rcvd = '\0';

    while(1)
    {
        if(!queue_is_empty(&rx_fifo))
        {
            queue_try_remove(&rx_fifo, &rcvd);
            cout << ">>" << rcvd << endl;
        }
        sleep_us(100);
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
