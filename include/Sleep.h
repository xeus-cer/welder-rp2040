#ifndef __SLEEP_H
#define __SLEEP_H


#include "xerxes_rp2040.h"
#include "ClockUtils.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"


// watchdog friendly sleep in low power mode
void sleep_lp(uint64_t us)
{
    // disable communication
    gpio_put(RS_EN_PIN, 0);

    // disable USR_LED
	gpio_set_dir(USR_LED_PIN, GPIO_IN);

    // lower speed and voltage on both cores
    setClocksLP();    
    
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
    setClocksHP();

    // enable USR_LED
    gpio_set_dir(USR_LED_PIN, GPIO_OUT);

    // resume communication
    gpio_put(RS_EN_PIN, 1);
}


// sleep with watchdog enabled for usb mode
void sleep_hp(uint64_t us)
{
    // waste some time - keep watchdog updated 
    while(us + 1000 > DEFAULT_WATCHDOG_DELAY * 1000)
    {
        // watchdog would reset the chip, split the sleep to smaller pieces
        sleep_us(DEFAULT_WATCHDOG_DELAY * 500); // delay in ms * 1000 /2 for safety margin
        us -= DEFAULT_WATCHDOG_DELAY * 500;
        watchdog_update();
    }   
    sleep_us(us);
}


#endif // !__SLEEP_H