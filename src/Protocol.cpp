#include "Protocol.hpp"

namespace Xerxes
{


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