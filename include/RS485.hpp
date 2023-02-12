#ifndef __RS485_HPP
#define __RS485_HPP


#include "Network.hpp"
#include "pico/util/queue.h"
#include "Packet.hpp"
#include "Message.hpp"

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

    
class RS485 : public Network
{
private:
    queue_t *qtx;
    queue_t *qrx;
    std::vector<uint8_t> incomingMessage {};

public:
    RS485(queue_t *queueTx, queue_t *queueRx);
    ~RS485();

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


    Packet parsePacket();
};


} // namespace Xerxes


#endif // !__RS485_HPP
