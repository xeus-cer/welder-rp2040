#ifndef __USER_FLASH_HPP


/**
 * @brief Read flash and copy to RAM
 * 
 * @return true if flash is not empty and some data was read
 * @return false if flash is empty
 */
bool userInitFlash();


/**
 * @brief Update flash with current memory contents
 * 
 */
void updateFlash();


#endif // !__USER_FLASH_HPP