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


} // namespace Xerxes


#endif // !__PROTOCOL_HPP