#ifndef __PROTOCOL_HPP
#define __PROTOCOL_HPP

#include "RS485.hpp"

namespace Xerxes
{
    
class Protocol
{
private:
    Network *xn;
public:
    Protocol(Network *network);
    ~Protocol();

    void sendMessage(const uint8_t address, uint8_t *data);
    void readMessage(const uint8_t *rcvBuf, const uint32_t timeoutMs);
};

Protocol::Protocol(Network *network) : xn(network)
{
}

Protocol::~Protocol()
{
}


} // namespace Xerxes


#endif // !__PROTOCOL_HPP