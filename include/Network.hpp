#ifndef __NETWORK_HPP
#define __NETWORK_HPP

#include "Packet.hpp"

namespace Xerxes
{
    
class Network
{
private:
    //
public:
    Network(/* args */);
    ~Network();

    virtual uint16_t sendData(const Packet & toSend);
    virtual uint16_t readData(uint8_t *receiveBuffer, uint32_t timeoutMs);
};

Network::Network(/* args */)
{
}

Network::~Network()
{
}


} // namespace Xerxes


#endif // !__NETWORK_HPP
