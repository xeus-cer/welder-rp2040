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


RS485::RS485(queue_t *queueTx, queue_t *queueRx) : qtx(queueTx), qrx(queueRx)
{
}


RS485::~RS485()
{
}


bool RS485::sendData(const Packet & toSend) const
{
    // improve: this can check if queue has enough space before trying to send data
    uint16_t sent {0};
    for(const auto &el:toSend.getData())
    {
        if(!queue_try_add(qtx, &el)) break;
        sent++; // byte was sent successfully
    }

    
    // return true if whole packet was sent
    return sent == toSend.size();
}


bool RS485::readData(const uint64_t timeoutUs, Packet &packet)
{
    // start timer
    uint64_t start = time_us_64();

    // if RX queue is empty, immediately return
    if(queue_is_empty(qrx))
    {
        return false;
    }

    if(receivePacket(timeoutUs))
    {
        packet = Packet(incomingMessage);
        return true;
    }

    return false;
}


bool RS485::receivePacket(const uint64_t timeoutUs)
{
    // check if the packet is in fifo buffer
    uint8_t nextVal = 0;
    
    #ifdef NDEBUG
    uint64_t tout = time_us_64() + timeoutUs;
    #else
    absolute_time_t tout;
    tout._private_us_since_boot = time_us_64() + timeoutUs;
    #endif // NDEBUG

    uint8_t chks = SOH;

    uint8_t msgLen = 0;
    volatile bool waitForSoh = true;

    // clear buffer
    incomingMessage.clear();

    while(!time_reached(tout))
    {
        if(waitForSoh)
        {
            if(queue_try_remove(qrx, &nextVal))
            {
                if(nextVal == SOH)
                {
                    waitForSoh = false;
                }
            }
        }
        else
        {
            // SOH was found, get msgLen
            if(queue_try_remove(qrx, &msgLen))
            {
                // add msglen to checksum
                chks += msgLen;
                break; //break from this loop, continue to receive all data
            }
        }
    }

    msgLen -= 3; // SOH and len was received, checksum is received at the end hence -3
    while(!time_reached(tout) && msgLen)
    {
        if(queue_try_remove(qrx, &nextVal))
        {
            // something was received, slap it to the incoming vector
            incomingMessage.emplace_back(nextVal);
            chks += nextVal;
            msgLen--;
        }
    }

    while(!time_reached(tout))
    {
        //wait for checksum byte
        if(queue_try_remove(qrx, &nextVal))
        {
            chks += nextVal;
            if(chks == 0)
            {
                // successfully received whole message
                return true;
            }   
            else
            {
                return false;
            }
        }
    }

    return false;
}


uint32_t remainingTime(const uint64_t & start, const uint64_t &timeout)
{
    auto current_time = time_us_64();
    return timeout - (current_time - start);
}


} // namespace Xerxes


#endif // !__RS485_HPP
