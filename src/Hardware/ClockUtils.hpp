#ifndef __CLOCK_UTILS_H
#define __CLOCK_UTILS_H


/**
 * @brief Initialize clocks to default values
 * 
 */
void userInitClocks();


/**
 * @brief Set system clock to default values
 * 
 */
void setClockSysDefault();


/**
 * @brief Set clock for ADC to default values
 * 
 */
void setClockAdcDefault();


/**
 * @brief Set clock for peripherals to default values
 * 
 */
void setClockPeriDefault();


/**
 * @brief Initialize PLL_USB
 * 
 */
void setClockUsbDefault();


/**
 * @brief Set system clock to low power values
 * 
 */
void setClockSysLP();   


/**
 * @brief Measure and print frequencies of all clocks
 * 
 */
void measure_freqs(void);


/**
 * @brief Set all clocks to low power values
 * 
 */
void setClocksLP();


/**
 * @brief Set all clocks to high power values
 * 
 */
void setClocksHP();


#endif // !__CLOCK_UTILS_H    