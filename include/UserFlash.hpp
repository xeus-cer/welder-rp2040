#ifndef __USER_FLASH_HPP


#include "hardware/flash.h"
#include "hardware/sync.h"
#include <cstring>

#include "Memory.hpp"


void userInitFlash()
{
    auto status = save_and_disable_interrupts();

    //read UID
    flash_get_unique_id((uint8_t *)uid);

    // std::memcpy(&config, &flash_target_contents[OFFSET_CONFIG_BITS], 1);
    // it takes approx 750us to program 256bytes of flash
    std::memcpy((uint8_t *)mainRegister, flash_target_contents, NON_VOLATILE_SIZE);
    
    //erase flash, must be done in sector size
    // flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
}


void updateFlash()
{
    // disable interrupts first
    auto status = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, NON_VOLATILE_SIZE);

    // write flash, must be done in page size
    // it takes approx 450us to write 128bytes of data
    uint8_t memImage[NON_VOLATILE_SIZE];
    std::memcpy(memImage, (uint8_t *)mainRegister, sizeof(memImage));
    flash_range_program(FLASH_TARGET_OFFSET, memImage, NON_VOLATILE_SIZE);

    restore_interrupts(status);
}


#endif // !__USER_FLASH_HPP