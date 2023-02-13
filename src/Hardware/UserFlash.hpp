#ifndef __USER_FLASH_HPP

#include <stdint.h>


/**
 * @brief Read flash and copy to RAM
 * 
 * @return true if flash is not empty and some data was read
 * @return false if flash is empty
 */
bool userInitFlash(uint8_t *memTable);


/**
 * @brief Update flash with current memory contents
 * 
 */
void updateFlash(const uint8_t *memTable);


#endif // !__USER_FLASH_HPP