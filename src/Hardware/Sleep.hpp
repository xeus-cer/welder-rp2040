#ifndef __SLEEP_H
#define __SLEEP_H

#include <cstdint>


/** @brief watchdog friendly sleep in low power mode */
void sleep_lp(uint64_t us);


/** @brief sleep with watchdog enabled for usb mode */
void sleep_hp(uint64_t us);


#endif // !__SLEEP_H