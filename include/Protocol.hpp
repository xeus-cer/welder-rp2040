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
    bool readMessage(Message &message, const uint64_t timeoutUs);
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


bool Protocol::readMessage(Message &message, const uint64_t timeoutUs)
{
    Packet packet = Packet();
    
    if(xn->readData(timeoutUs, packet))
    {        
        message = Message(packet);
        return true;
    }

    return false;
}


} // namespace Xerxes


#endif // !__PROTOCOL_HPP