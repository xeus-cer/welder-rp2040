#ifndef __RS485_HPP
#define __RS485_HPP


#include <Network.hpp>
#include "pico/util/queue.h"
#include <Packet.hpp>
#include <Message.hpp>

#include <stdexcept>


namespace Xerxes
{


/**
 * @brief Get amount of time remaining from start to timeout
 * 
 * @param start start time in us
 * @param timeout in us
 * @return uint16_t remaining time in us
 */
uint32_t remainingTime(const uint64_t & start, const uint64_t &timeout);
    

/**
 * @brief RS485 class for communication over RS485
 * 
 * This class is used to implement the RS485 communication. 
 */
class RS485 : public Network
{
private:
    /// @brief Pointer to the queue for sending data
    queue_t *qtx;
    /// @brief Pointer to the queue for receiving data
    queue_t *qrx;
    /// @brief Buffer for incoming data
    std::vector<uint8_t> incomingMessage {};

public:
    /**
     * @brief Construct a new RS485 object
     * 
     * @param queueTx queue for sending data
     * @param queueRx queue with received data
     */
    RS485(queue_t *queueTx, queue_t *queueRx);
    ~RS485();

    /**
     * @brief send one Packet over the network
     * 
     * @param toSend packet to send
     * @return true if the packet was sent successfully
     * @return false if the packet was not sent successfully
     */
    bool sendData(const Packet & toSend) const;


    /**
     * @brief read one Packet from the network
     * 
     * @param timeoutUs timeout in us
     * @return Packet 
     */
    bool readData(const uint64_t timeoutUs, Packet &packet);
    
    /**
     * @brief check whether there is valid packet in the buffer
     * 
     * @return true valid packet awaits in the incoming buffer
     * @return false otherwise
     */
    bool receivePacket(const uint64_t timeoutUs);


    /**
     * @brief parse the packet in the buffer
     * 
     * @return Packet 
     */
    Packet parsePacket();
};


} // namespace Xerxes


#endif // !__RS485_HPP
