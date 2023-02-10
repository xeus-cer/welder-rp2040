#include "UserFlash.hpp"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include <cstring>
#include "Definitions.h"


extern volatile uint8_t *mainRegister;
extern const uint8_t *flash_target_contents;
extern uint64_t* uid;


bool userInitFlash()
{
    // disable interrupts first
    auto status = save_and_disable_interrupts();

    //read UID
    flash_get_unique_id((uint8_t *)uid);

    // read flash
    std::memcpy((uint8_t *)mainRegister, flash_target_contents, VOLATILE_OFFSET);

    // check if flash is empty
    bool empty = true;
    for(uint16_t i = 0; i < VOLATILE_OFFSET; i++)
    {
        if(mainRegister[i] != 0xFF)
        {
            empty = false;
            break;
        }
    }
       
    restore_interrupts(status);
    if(empty) return false;
    else return true;
}


void updateFlash()
{
    // disable interrupts first
    auto status = save_and_disable_interrupts();

    // erase flash, must be done in sector size, 4KB, it takes 49ms    
    flash_range_erase(FLASH_TARGET_OFFSET, VOLATILE_OFFSET);

    // copy memory to buffer
    uint8_t memImage[VOLATILE_OFFSET];
    std::memcpy(memImage, (uint8_t *)mainRegister, sizeof(memImage));

    // write flash, must be done in page size (256bytes), approx 400us
    flash_range_program(FLASH_TARGET_OFFSET, memImage, VOLATILE_OFFSET);

    // finally, restore interrupts
    restore_interrupts(status);
}