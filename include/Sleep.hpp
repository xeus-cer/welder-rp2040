#ifndef __SLEEP_HPP


#include "xerxes_rp2040.h"
#include "ClockUtils.hpp"

void sleep_lp(uint64_t us)
{
    // disable communication
    gpio_put(RS_EN_PIN, 0);

    // lower speed and voltage on both cores
    setClockSysLP();    
    
    // waste some time - keep watchdog updated
    while(us + 1000 > DEFAULT_WATCHDOG_DELAY * 1000)
    {
        // watchdog would reset the chip, split the sleep to smaller pieces
        sleep_us(DEFAULT_WATCHDOG_DELAY * 500); // delay in ms * 1000 /2 for safety margin
        us -= DEFAULT_WATCHDOG_DELAY * 500;
        watchdog_update();
    }   
    sleep_us(us);

    // raise the speed back to normal
    setClockSysDefault();     
    // resume communication
    gpio_put(RS_EN_PIN, 1);
}


#endif // !__SLEEP_HPP