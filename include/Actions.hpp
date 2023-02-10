#ifndef __ACTIONS_HPP


#include <functional>
#include "UserFlash.hpp"
#include "DeviceIds.h"
#include "Sleep.hpp"
#include "InitUtils.hpp"
#include "hardware/watchdog.h"


extern Xerxes::Slave xs;
extern void pollSensor();


/**
 * @brief Decorate a function to be unicast targeted
 * 
 * a function is called only on targeted unicast packet
 * 
 * @tparam Func 
 * @param f function to call for unicast packets
 * @return std::function<void(const Xerxes::Message &)> decorated lambda function
 */
template <typename Func>
std::function<void(const Xerxes::Message &)> unicast(Func f) 
{
  // The returned function is a lambda function that takes the same arguments as the original
  // function and calls it with the given arguments.
    return [f](const Xerxes::Message &msg) {
        if(msg.dstAddr!= 0xff && *devAddress == msg.dstAddr)
        {
        // Call the original function with the given arguments.
            f(msg);
        }
        else
        {       
            // do nothing
        }
    };
}


/**
 * @brief Decorate a function to be broadcast targeted
 * 
 * a function is called on targeted unicast packet or broadcast packet
 * 
 * @tparam Func 
 * @param f function to call for unicast packets
 * @return std::function<void(const Xerxes::Message &)> decorated lambda function
 */
template <typename Func>
std::function<void(const Xerxes::Message &)> broadcast(Func f) 
{
    return [f](const Xerxes::Message &msg) {
        if(msg.dstAddr == 0xff || *devAddress == msg.dstAddr)
        {
        // Call the original function with the given arguments.
            f(msg);
        }
        else
        {       
            // do nothing
        }
    };
}


/**
 * @brief Ping callback
 * 
 * Reply to ping request with device id and protocol version
 * 
 * @param msg incoming message
 */
void pingCallback(const Xerxes::Message &msg)
{
    std::vector<uint8_t> payload {DEVID_PRESSURE_60MBAR, PROTOCOL_VERSION_MAJ, PROTOCOL_VERSION_MIN};
    xs.send(msg.srcAddr, MSGID_PING_REPLY, payload);
}


/**
 * @brief Synchronize callback
 * 
 * Used to simultaneously poll all sensors on the bus
 * 
 * @param msg incoming message
 * 
 * @note This function does not return an answer, it only polls the sensor
 */
void syncCallback(const Xerxes::Message &msg)
{
    pollSensor();
}


/**
 * @brief Write register callback
 * 
 * Write <LEN> bytes of <DATA> to the device register, starting at <REG_ID>
 * The request prototype is <MSGID_WRITE> <REG_ID> <LEN> <DATA>
 * 
 * @note This function is blocking, it will not return until the data is written to the register
 * 
 * @param msg 
 */
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
    updateFlash();
    watchdog_update();

    // send ACK_OK
    xs.send(msg.srcAddr, MSGID_ACK_OK);
}


/**
 * @brief Read register callback
 * 
 * Read <LEN> bytes from device register, starting at <REG_ID>
 * The request prototype is <MSGID_READ> <REG_ID> <LEN>
 * 
 * @param msg 
 * 
 * @note This function is blocking, it will not return until the data is read from the register
 * and sent to the master device.
 * @note All data are in little endian format - LSB first. 
 */
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


/**
 * @brief Attempt to perform low power sleep
 * 
 * @param msg incoming message
 */
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

/**
 * @brief Attempt to perform soft reset
 * 
 * @param msg
 * 
 * @note This function does not return
 */
void softResetCallback(const Xerxes::Message &msg)
{
    watchdog_reboot(0,0,0);
}


/**
 * @brief Attempt to perform factory reset
 * 
 * @note This function may be called only if memory is unlocked by writing the correct value to the memUnlocked register
 * 
 * @param msg 
 */
void factoryResetCallback(const Xerxes::Message &msg)
{   
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


#endif // !__ACTIONS_HPP