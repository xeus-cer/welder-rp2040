#ifndef __SLAVE_HPP
#define __SLAVE_HPP

#include "Protocol.hpp"
#include <unordered_map>
#include <functional>

namespace Xerxes
{

void writeReg(const uint8_t source);
void readReg(const uint8_t source);


class Slave
{
private:
    Protocol *xp;
    std::unordered_map<msgid_t, std::function<void(uint8_t)>> bindings;
    uint8_t address;

public:
    Slave(Protocol *protocol, const uint8_t address);
    ~Slave();
    void bind(const msgid_t msgId, std::function<void(uint8_t)> _f);
    void call(const msgid_t msgId, const uint8_t sourceAddress);

    bool send(const uint8_t destinationAddress, const msgid_t msgId);
    bool send(const uint8_t destinationAddress, const msgid_t msgId, const std::vector<uint8_t> &payload);
    bool sync(const uint32_t timeoutUs);
};

Slave::Slave(Protocol *protocol, const uint8_t address) : xp(protocol), address(address)
{
    bind(MSGID_READ, readReg); //TODO toto nemoze ist cez bindy 
}

Slave::~Slave()
{
}


void Slave::bind(const msgid_t msgId, std::function<void(uint8_t)> _f)
{
    bindings.emplace(msgId, _f);
}


void Slave::call(const msgid_t msgId, const uint8_t sourceAddress) 
{
    if(bindings.contains(msgId)){
        // call a function binded to messageId
        bindings[msgId](sourceAddress);
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

    // if no message is in buffer
    Message ingress = Message();
    if(!xp->readMessage(&ingress, timeoutUs))
    {
        return false;
    }
            
    // read msgId
    uint8_t sourceAddr = ingress.srcAddr;
    msgid_t msgId = ingress.msgId;
    
    // call appropriate function
    call(msgId, sourceAddr);
    return true;
}
    

} // namespace Xerxes

#endif // !__SLAVE_HPP