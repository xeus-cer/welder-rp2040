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
    gpio_put(RS_EN_PIN, 0);
    // gpio_put(EXT_3V3_EN_PIN, 0);
    // config->bits.freeRun = 0;

    sleep_us(us);

    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        125 * MHZ,
        125 * MHZ
    );
    
    gpio_put(RS_EN_PIN, 1);
    // gpio_put(EXT_3V3_EN_PIN, 1);
    // config->bits.freeRun = _fr;
    // change it back
}


#endif // !__SLEEP_HPP