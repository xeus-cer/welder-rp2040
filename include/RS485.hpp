#ifndef __RS485_HPP
#define __RS485_HPP

#include "Network.hpp"
#include "pico/util/queue.h"
#include "Packet.hpp"

namespace Xerxes
{

    
class RS485 : Network
{
private:
    queue_t *qtx;
    queue_t *qrx;
public:
    RS485(queue_t *queueTx, queue_t *queueRx);
    ~RS485();

    bool sendData(Packet &toSend);
    uint16_t readData(uint8_t *receiveBuffer, uint32_t timeoutMs);
};

RS485::RS485(queue_t *queueTx, queue_t *queueRx) : qtx(queueTx), qrx(queueRx)
{
}

RS485::~RS485()
{
}


bool RS485::sendData(Packet &toSend)
{

    if(!queue_try_add(qtx, &toSend.soh)) return false;
    if(!queue_try_add(qtx, &toSend.msgLen)) return false;

    for(const auto &el:toSend.data)
    {
        if(!queue_try_add(qtx, &el)) return false;
    }

    return true;
}


uint16_t RS485::readData(uint8_t *receiveBuffer, uint32_t timeoutMs)
{
    // start timer
    uint64_t start = time_us_64();
    uint16_t rcvdln {0};
    do
    {
        if(queue_is_empty(qrx))
        {
            // nothing to receive, wait a bit
            sleep_us(100);
        }
        else
        {
            // remove one element
            queue_try_remove(qrx, &receiveBuffer[rcvdln++]);
        }
    }
    while((time_us_64() - start) < (timeoutMs * 1000));

    return rcvdln;
}


} // namespace Xerxes


#endif // !__RS485_HPP
