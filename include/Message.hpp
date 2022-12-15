#ifndef __MESSAGE_HPP
#define __MESSAGE_HPP

#include <array>
#include <vector>
#include "Packet.hpp"

namespace Xerxes
{
    
class Message
{
private:
    std::vector<uint8_t> messageBytes {};
public:
    uint8_t srcAddr;
    uint8_t dstAddr;
    uint16_t msgId;
    Message(const uint8_t source, const uint8_t destination, const uint16_t msgid, const std::vector<uint8_t> &payload);
    Message(const uint8_t source, const uint8_t destination, const uint16_t msgid);
    Message();
    ~Message();
    void update(const uint8_t source, const uint8_t destination, const uint16_t msgid);

    Packet toPacket();

};


Message::Message()
{
}


Message::Message(
    const uint8_t source, 
    const uint8_t destination, 
    const uint16_t msgid, 
    const std::vector<uint8_t> &payload
    ) : srcAddr(source), dstAddr(destination), msgId(msgid)
{

    messageBytes.emplace_back(srcAddr);
    messageBytes.emplace_back(dstAddr);
    messageBytes.emplace_back(msgId % 0x100); // low byte
    messageBytes.emplace_back(msgId >> 8); // high byte - because of little endianness

    for(const auto &el: payload)
    {
        messageBytes.emplace_back(el);
    }
}


Message::Message(
    const uint8_t source, 
    const uint8_t destination, 
    const uint16_t msgid
    ) : srcAddr(source), dstAddr(destination), msgId(msgid)
{
    messageBytes.emplace_back(srcAddr);
    messageBytes.emplace_back(dstAddr);
    messageBytes.emplace_back(msgId % 0x100); // low byte
    messageBytes.emplace_back(msgId >> 8); // high byte - because of little endianness
}


Message::~Message()
{

}

Packet Message::toPacket()
{
    return Packet(messageBytes);
}


void Message::update(
    const uint8_t source, 
    const uint8_t destination, 
    const uint16_t msgid
){
    srcAddr = source;
    dstAddr = destination;
    msgId = msgid;
    messageBytes.emplace_back(srcAddr);
    messageBytes.emplace_back(dstAddr);
    messageBytes.emplace_back(msgId % 0x100); // low byte
    messageBytes.emplace_back(msgId >> 8); // high byte - because of little endianness
}

} // namespace Xerxes

#endif // !__MESSAGE_HPP