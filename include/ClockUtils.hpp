#ifndef __CLOCK_UTILS_HPP
#define __CLOCK_UTILS_HPP

#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "Definitions.h"
#include "hardware/vreg.h"


void setClockAdcDefault()
{
    clock_configure(
        clk_adc,
        0,
        DEFAULT_ADC_CLOCK_SRC,
        DEFAULT_ADC_CLOCK_FREQ,
        DEFAULT_ADC_CLOCK_FREQ
    );
}


void setClockPeriDefault()
{
    // change peripheral clock to lower frequency - 48MHz
    clock_configure(
        clk_peri,
        0,
        DEFAULT_PERI_CLOCK_SRC,
        DEFAULT_PERI_CLOCK_FREQ,
        DEFAULT_PERI_CLOCK_FREQ
    );
}


void setClockUsbDefault()
{
    pll_init(pll_usb, 1, 480 * MHZ, 5, 2);

    clock_configure(
        clk_usb, 
        0, 
        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 
        48 * MHZ, 
        48 * MHZ
    );
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

    // pll_sys  = 125001kHz
    // pll_usb  = 48001kHz
    // rosc     = 5321kHz
    // clk_sys  = 125000kHz
    // clk_peri = 125000kHz
    // clk_usb  = 48000kHz
    // clk_adc  = 48000kHz
    // clk_rtc  = 47kHz
 
    // Can't measure clk_ref / xosc as it is the ref
}


void setClockSysLP()
{
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_XOSC_CLKSRC,
        DEFAULT_XOSC_CLOCK_FREQ,
        DEFAULT_XOSC_CLOCK_FREQ
    );
    pll_deinit(pll_sys);
    vreg_set_voltage(VREG_VOLTAGE_0_95);
}


void setClockSysDefault()
{
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_XOSC_CLKSRC,
        DEFAULT_XOSC_CLOCK_FREQ,
        DEFAULT_XOSC_CLOCK_FREQ
    );
    // change voltage of VReg to default value
    vreg_set_voltage(DEFAULT_SYS_VOLTAGE);

    // Reconfigure PLL sys back to the default state
    pll_init(pll_sys, 1, DEFAULT_SYS_PLL_FREQ, DEFAULT_SYS_PLL_POST_1, DEFAULT_SYS_PLL_POST_2);
    
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        DEFAULT_SYS_CLOCK_SRC,
        DEFAULT_SYS_CLOCK_FREQ,
        DEFAULT_SYS_CLOCK_FREQ
    );
}


void userInitClocks()
{
    setClockSysDefault();
    setClockAdcDefault();
    setClockPeriDefault();
}


#endif // !__CLOCK_UTILS_HPP    