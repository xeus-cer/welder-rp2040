#include "Callbacks.hpp"


#include "UserFlash.hpp"
#include "DeviceIds.h"
#include "Sleep.hpp"
#include "InitUtils.hpp"
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "Definitions.h"
#include "Slave.hpp"


extern Xerxes::Slave xs;
extern void pollSensor();
extern uint8_t mainRegister[REGISTER_SIZE];


namespace Xerxes
{


void pingCallback(const Xerxes::Message &msg)
{
    std::vector<uint8_t> payload {DEVID_PRESSURE_60MBAR, PROTOCOL_VERSION_MAJ, PROTOCOL_VERSION_MIN};
    xs.send(msg.srcAddr, MSGID_PING_REPLY, payload);
}


void syncCallback(const Xerxes::Message &msg)
{
    pollSensor();
}


void writeRegCallback(const Xerxes::Message &msg)
{   
    // read offset from message
    uint8_t offsetL = msg.at(4);
    uint8_t offsetH = msg.at(5);
    // convert to uint16_t
    uint16_t offset = (offsetH << 8) + offsetL;
        
    // check if offset is valid (not read only memory and not negative)
    if(offset >= READ_ONLY_OFFSET || offset < 0)
    {
        // send ACK_NOK
        xs.send(msg.srcAddr, MSGID_ACK_NOK);
        return;
    }

    // lock out core1, wait 10ms for core1 to lock out
    if(!multicore_lockout_start_timeout_us(10'000))
    {
        // lockout failed, send ACK_NOK
        xs.send(msg.srcAddr, MSGID_ACK_NOK);
        return;
    }

    // disable interrupts
    auto status = save_and_disable_interrupts();

    // write data to memory
    for(uint16_t i = 6; i < msg.size(); i++)
    {
        uint8_t byte = msg.at(i);
        mainRegister[offset + i - 6] = byte;
    }

    // restore interrupts
    restore_interrupts(status);

    // unlock core1, wait 10ms for core1 to unlock
    multicore_lockout_end_timeout_us(10'000);
    
    // update flash, takes ~50ms to complete hence the 2 watchdog updates
    watchdog_update();
    updateFlash((uint8_t *)mainRegister);
    watchdog_update();

    // send ACK_OK
    xs.send(msg.srcAddr, MSGID_ACK_OK);
}


void readRegCallback(const Xerxes::Message &msg)
{
    // read offset from message in little endian
    uint8_t offsetL = msg.at(4);
    uint8_t offsetH = msg.at(5);
    // convert to uint16_t
    uint16_t offset = (offsetH << 8) + offsetL;

    // read length to read from message (num of bytes to read)
    uint8_t len = msg.at(6);

    // check if offset and length are valid (not longer than register size)
    if(offset + len > REGISTER_SIZE)
    {
        // send ACK_NOK
        xs.send(msg.srcAddr, MSGID_ACK_NOK);
        return;
    }
    
    std::vector<uint8_t> payload {};

    // read data from memory into payload vector
    for(uint16_t i = offset; i < offset + len; i++)
    {
        payload.emplace_back(mainRegister[i]);
    }

    // send data to master device (MSGID_READ_VALUE + payload) 
    xs.send(msg.srcAddr, MSGID_READ_VALUE, payload);

}


void sleepCallback(const Xerxes::Message &msg)
{
    uint8_t raw_duration[4];
    
    for(uint8_t i = 0; i < 4; i++)
    {
        raw_duration[i] = msg.at(i + 4);
    }

    uint32_t *durationUs = (uint32_t *)raw_duration;
    uint64_t cleanUs = static_cast<uint64_t>(*durationUs);
    
    sleep_lp(cleanUs);
}


void softResetCallback(const Xerxes::Message &msg)
{
    watchdog_reboot(0,0,0);
}


void factoryResetCallback(const Xerxes::Message &msg)
{   
    /** @brief 0x55AA55AA = unlocked, anything else = locked */
    uint32_t* memUnlocked   = (uint32_t *)(mainRegister + MEM_UNLOCKED_OFFSET);

    // check if memory is unlocked (factory reset is allowed only if memory is unlocked)
    if(*memUnlocked == MEM_UNLOCKED_VAL)
    {
        // reset memory
        userLoadDefaultValues();
        // reset device
        watchdog_reboot(0,0,0);
    }
    else
    {
        // send ACK_NOK
        xs.send(msg.srcAddr, MSGID_ACK_NOK);
        return;
    }
}


} // namespace Xerxes