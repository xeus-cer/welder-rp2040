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

    bool sendMessage(Message &message) const;
    bool readMessage(Message *message, const uint32_t timeoutUs);
};

Protocol::Protocol(Network *network) : xn(network)
{
}

Protocol::~Protocol()
{
}

bool Protocol::sendMessage(Message &message) const
{
    return xn->sendData(message.toPacket());
}


bool Protocol::readMessage(Message *message, const uint32_t timeoutUs)
{
    uint16_t received {0};
    
    uint8_t source, destination;
    msgid_t msgId;
    //TODO Magick

    message->update(source, destination, msgId);
    return true;
}


} // namespace Xerxes


#endif // !__PROTOCOL_HPP