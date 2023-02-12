#ifndef __CALLBACKS_HPP


#include <functional>
#include "Message.hpp"


namespace Xerxes
{


/**
 * @brief Ping callback
 * 
 * Reply to ping request with device id and protocol version
 * 
 * @param msg incoming message
 */
void pingCallback(const Xerxes::Message &msg);


/**
 * @brief Synchronize callback
 * 
 * Used to simultaneously poll all sensors on the bus
 * 
 * @param msg incoming message
 * 
 * @note This function does not return an answer, it only polls the sensor
 */
void syncCallback(const Xerxes::Message &msg);


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
void writeRegCallback(const Xerxes::Message &msg);


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
void readRegCallback(const Xerxes::Message &msg);


/**
 * @brief Attempt to perform low power sleep
 * 
 * @param msg incoming message
 */
void sleepCallback(const Xerxes::Message &msg);


/**
 * @brief Attempt to perform soft reset
 * 
 * @param msg
 * 
 * @note This function does not return
 */
void softResetCallback(const Xerxes::Message &msg);


/**
 * @brief Attempt to perform factory reset
 * 
 * @note This function may be called only if memory is unlocked by writing the correct value to the memUnlocked register
 * 
 * @param msg 
 */
void factoryResetCallback(const Xerxes::Message &msg);


} // namespace Xerxes


#endif // !__CALLBACKS_HPP