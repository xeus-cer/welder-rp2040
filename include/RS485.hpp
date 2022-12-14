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
uint32_t remainingTime(const uint32_t & start, const uint32_t &timeout);

    
class RS485 : Network
{
private:
    queue_t *qtx;
    queue_t *qrx;
    std::vector<uint8_t> incomingMessage {};

public:
    RS485(queue_t *queueTx, queue_t *queueRx);
    ~RS485();

    uint16_t sendData(const Packet & toSend);


    /**
     * @brief read one Packet from the network
     * 
     * @param timeoutUs timeout in us
     * @return Packet 
     */
    bool readData(const uint32_t timeoutUs, Packet * packet);
    
    /**
     * @brief check whether there is valid packet in the buffer
     * 
     * @return true valid packet awaits in the incoming buffer
     * @return false otherwise
     */
    bool receivePacket(const uint32_t timeoutUs);


    Packet parsePacket();
};


RS485::RS485(queue_t *queueTx, queue_t *queueRx) : qtx(queueTx), qrx(queueRx)
{
}


RS485::~RS485()
{
}


uint16_t RS485::sendData(const Packet & toSend)
{
    uint16_t sent {0};
    for(const auto &el:toSend.getData())
    {
        if(!queue_try_add(qtx, &el)) break;
        sent++; // byte was sent successfully
    }

    return sent;
}


bool RS485::readData(const uint32_t timeoutUs, Packet * packet)
{
    // start timer
    uint64_t start = time_us_64();

    // clear the message buffer
    incomingMessage.clear();

    while(
        !time_reached(start + timeoutUs) && 
        !receivePacket(remainingTime(start, timeoutUs))
    )
    {
        sleep_us(100);
    }

    *packet = Packet(incomingMessage);

    return false;
}


bool RS485::receivePacket(const uint32_t timeoutUs)
{
    // check if the packet is in fifo buffer
    uint8_t nextVal;

    uint64_t start = time_us_64();
    uint8_t chks = SOH;

    uint8_t msgLen;
    bool waitForSoh = true;

    while(!time_reached(start + timeoutUs))
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
                chks += msgLen;
                break; //break from this loop, continue to receive all data
            }
        }
    }

    while(!time_reached(start + timeoutUs) && msgLen)
    {
        if(queue_try_remove(qrx, &nextVal))
        {
            // something was received, slap it to the incoming vector
            incomingMessage.emplace_back(nextVal);
            chks += nextVal;
            msgLen--;
        }
    }

    while(!time_reached(start + timeoutUs))
    {
        //wait for checksum byte

        if(queue_try_remove(qrx, &nextVal))
        {
            chks += nextVal;
            if(chks == 0)
            {
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


uint32_t remainingTime(const uint32_t & start, const uint32_t &timeout)
{
    auto current_time = time_us_64();
    return timeout - (current_time - start);
}


} // namespace Xerxes


#endif // !__RS485_HPP
