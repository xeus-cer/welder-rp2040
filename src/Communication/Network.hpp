#ifndef __NETWORK_HPP
#define __NETWORK_HPP

#include "Packet.hpp"


namespace Xerxes
{


constexpr uint8_t BROADCAST_ADDR = 0xff;

    
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


} // namespace Xerxes


#endif // !__NETWORK_HPP
