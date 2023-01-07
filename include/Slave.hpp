#ifndef __SLAVE_HPP
#define __SLAVE_HPP

#include "Protocol.hpp"
#include <unordered_map>
#include <functional>

namespace Xerxes
{


class Slave
{
private:
    Protocol *xp;
    std::unordered_map<msgid_t, std::function<void(const Message&)>> bindings;
    uint8_t address;
    volatile uint8_t *mainReg;

public:
    Slave();
    Slave(Protocol *protocol, const uint8_t address, volatile uint8_t *mainRegister);
    ~Slave();
    void bind(const msgid_t msgId, std::function<void(const Message&)> _f);
    void call(const Message &msg);

    bool send(const uint8_t destinationAddress, const msgid_t msgId);
    bool send(const uint8_t destinationAddress, const msgid_t msgId, const std::vector<uint8_t> &payload);
    bool sync(const uint32_t timeoutUs);
};


Slave::Slave()
{
}

Slave::Slave(Protocol *protocol, const uint8_t address, volatile uint8_t *mainRegister) : xp(protocol), address(address), mainReg(mainRegister)
{
}

Slave::~Slave()
{
}


void Slave::bind(const msgid_t msgId, std::function<void(const Message&)> _f)
{
    bindings.emplace(msgId, _f);
}


void Slave::call(const Message &msg) 
{
    if(bindings.contains(msg.msgId)){
        // call a function bound to messageId
        bindings[msg.msgId](msg);
    }
}


bool Slave::send(const uint8_t destinationAddress, const msgid_t msgId)
{
    Message message(address, destinationAddress, msgId);
    return xp->sendMessage(message);
}


bool Slave::send(const uint8_t destinationAddress, const msgid_t msgId, const std::vector<uint8_t> &payload)
{
    Message message(address, destinationAddress, msgId, payload);
    return xp->sendMessage(message);
}


bool Slave::sync(uint32_t timeoutUs)
{
    // check for incoming message

    Message incoming = Message();
    if(!xp->readMessage(incoming, timeoutUs))
    {
        // if no message is in buffer
        return false;
    }
    

    printf("Received:\n");
    for(int i=0; i<incoming.size(); i++)
    {
        printf("%X", incoming.at(i));
    }
    printf("\n");
    
    // call appropriate function
    call(incoming);
    return true;
}

} // namespace Xerxes

#endif // !__SLAVE_HPP