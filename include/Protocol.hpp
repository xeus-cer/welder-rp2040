#ifndef __PROTOCOL_HPP
#define __PROTOCOL_HPP

#include "RS485.hpp"
#include "Message.hpp"

namespace Xerxes
{
    
class Protocol
{
private:
    Network *xn;
public:
    Protocol(Network *network);
    ~Protocol();

    uint16_t sendMessage(const Message &message);
    uint16_t readMessage(const uint8_t *rcvBuf, const uint32_t timeoutMs);
};

Protocol::Protocol(Network *network) : xn(network)
{
}

Protocol::~Protocol()
{
}

uint16_t Protocol::sendMessage(const Message &message)
{
    return xn->sendData(message.toPacket());
}


uint16_t Protocol::readMessage(const uint8_t *rcvBuf, const uint32_t timeoutMs)
{
    uint16_t received {0};
    //TODO Magick
    return received;
}


} // namespace Xerxes


#endif // !__PROTOCOL_HPP