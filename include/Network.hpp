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

    virtual bool sendData(const Packet & toSend) const = 0;
    virtual bool readData(const uint32_t timeoutUs, Packet * packet) = 0;
};

Network::Network(/* args */)
{
}

Network::~Network()
{
}


} // namespace Xerxes


#endif // !__NETWORK_HPP
