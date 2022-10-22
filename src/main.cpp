#include <iostream>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
//#include "pico/sleep.h"
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


using namespace std;


queue_t tx_fifo;
bool tx_needs_data = 0;
bool rx_has_data = 0;

 
void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
 
    /*
    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
    std::cout << std::endl << std::endl;
 */
    // Can't measure clk_ref / xosc as it is the ref
}

uint8_t this_dev_addr;

void initialize_uart(void)
{
    // Initialise UART 0 on 115200baud
    uart_init(uart0, 38400);
 
    // Set the GPIO pin mux to the UART - 16 is TX, 17 is RX
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);

    uart_set_fifo_enabled(uart0, 1);	
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


uint8_t uart0_read(){
    return uart_getc(uart0);
}


void uart0_write(uint8_t c){
    uart_putc_raw(uart0, c);
}


bool uart0_is_rx_ready()
{
    return uart_is_readable(uart0);
}


void fetch_handler(uint8_t c)
{
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(5);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}


void sleep_dormant()
{
    // UART will be reconfigured by sleep_run_from_xosc
    //sleep_run_from_xosc();

    printf("Running from XOSC\n");
    uart_default_tx_wait_blocking();

    printf("XOSC going dormant\n");
    uart_default_tx_wait_blocking();

    // Go to sleep until we see a high edge on GPIO 17 - RX
    // sleep_goto_dormant_until_edge_high(17);
    cout << "Pico woke" << endl;
}


void toggler()
{
    uint64_t i = 0;
    while(1)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        sleep_ms(500);
        queue_add_blocking(&tx_fifo, &i);
        i++;
    }
}

static uint PWM_0A = 0;
static uint PWM_1B = 3;

void pwm_user_init()
{

    gpio_init(PWM_0A);
    gpio_set_dir(PWM_0A, GPIO_OUT);
    gpio_init(PWM_1B);
    gpio_set_dir(PWM_1B, GPIO_IN);

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
 
    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);
    // Set period of 4 cycles (0 to 3 inclusive)
    //pwm_set_wrap(slice_num, 1023);

    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    //pwm_config_set_clkdiv(&config, 1);
    pwm_config_set_clkdiv_mode(&config, PWM_DIV_FREE_RUNNING);
    //pwm_config_set_wrap(&config, 12);
    // Set the PWM running
    pwm_init(slice_num, &config, true);
    //pwm_set_enabled(slice_num, true);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1<<15);

    uint slice2 = pwm_gpio_to_slice_num(2);

    pwm_config cfg2 = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg2, PWM_DIV_B_RISING);
    pwm_init(slice2, &cfg2, true);

}


void uart_interrupt_handler()
{
    rx_has_data = uart0_is_rx_ready();
    if(uart_is_writable(uart0) && queue_is_empty(&tx_fifo)){
        tx_needs_data = true;
    }
}


int main(void)
{
    this_dev_addr = 0x01;

	stdio_init_all();
    initialize_uart();
	
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    XerxesBusSetup(uart0_read, uart0_write, nop, nop, tx_done, uart0_is_rx_ready);

    XerxesDeviceSetup(DEVID_IO_8DI_8DO, fetch_handler, nop);

    queue_init(&tx_fifo, 8, 0x100);
    
    multicore_launch_core1(toggler);

    stdio_set_driver_enabled(&stdio_uart, false);

    irq_set_exclusive_handler(UART0_IRQ, uart_interrupt_handler);
    // enable uart interrupt for TX needs data, disable for RX has data
    uart_set_irq_enables(uart0, false, true);


	while (1)
	{
        
        XerxesSync();
        
        // measure_freqs();
        uint64_t rcv = 0;
        if(queue_try_remove(&tx_fifo, &rcv)) cout << rcv << endl;
        if(rx_has_data || tx_needs_data) cout << rx_has_data << tx_needs_data << endl;
        sleep_ms(100);
        // cout << config.csr << ", " << config.div << ", " << config.top << endl;
        // cout << "Counter: " << pwm_get_counter(slice2) << endl;
	}
}