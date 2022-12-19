#ifndef __NETWORK_HPP
#define __NETWORK_HPP

#include "Packet.hpp"

#define BROADCAST_ADDR  0xff

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
    virtual bool readData(const uint64_t timeoutUs, Packet &packet) = 0;
};

Network::Network(/* args */)
{
}

Network::~Network()
{
}


} // namespace Xerxes


#endif // !__NETWORK_HPP
