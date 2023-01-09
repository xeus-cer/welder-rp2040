#ifndef __SLEEP_HPP


#include "xerxes_rp2040.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

void sleep_lp(uint64_t us)
{
    // change clock to lower power

    // bool _fr = config->bits.freeRun;
    
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_XOSC_CLKSRC,
        12 * MHZ,
        12 * MHZ
    );

    // clock_configure(clk_peri, 0, 0, 0, 0);
    // clock_configure(clk_adc, 0, 0, 0, 0);
    // clock_configure(clk_usb, 0, 0, 0, 0);
    
    clock_stop(clk_adc);
    clock_stop(clk_peri);
    clock_stop(clk_usb);

    gpio_put(RS_EN_PIN, 0);
    // gpio_put(EXT_3V3_EN_PIN, 0);
    // config->bits.freeRun = 0;
    
    while(us + 1000 > DEFAULT_WATCHDOG_DELAY * 1000)
    {
        // watchdog would reset the chip, split the sleep to smaller pieces
        sleep_us(DEFAULT_WATCHDOG_DELAY * 500); // delay in ms * 1000 /2 for safety margin
        us -= DEFAULT_WATCHDOG_DELAY * 500;
        watchdog_update();
    }

    sleep_us(us);
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        125 * MHZ,
        125 * MHZ
    );

    clock_configure(
        clk_adc, 
        0, 
        DEFAULT_ADC_CLOCK_SRC,
        DEFAULT_ADC_CLOCK_FREQ, 
        DEFAULT_ADC_CLOCK_FREQ
    );

    clock_configure(
        clk_peri,
        0,
        DEFAULT_PERI_CLOCK_SRC,
        DEFAULT_PERI_CLOCK_FREQ,
        DEFAULT_PERI_CLOCK_FREQ
    );

    clock_configure(
        clk_usb, 
        0, 
        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 
        48 * MHZ, 
        48 * MHZ
    );

    gpio_put(RS_EN_PIN, 1);
    // gpio_put(EXT_3V3_EN_PIN, 1);
    // config->bits.freeRun = _fr;
    // change it back
}


#endif // !__SLEEP_HPP