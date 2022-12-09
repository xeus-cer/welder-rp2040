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
    std::vector<uint8_t> payload;
    Message(const uint8_t source, const uint8_t destination, const uint16_t msgid, std::vector<uint8_t> &payload);
    ~Message();

    Packet toPacket();
};


Message::Message(
    const uint8_t source, 
    const uint8_t destination, 
    const uint16_t msgid, 
    std::vector<uint8_t> &payload
    ) : srcAddr(source), dstAddr(destination), msgId(msgid)
{
    this->payload = payload;

    messageBytes.emplace_back(srcAddr);
    messageBytes.emplace_back(dstAddr);
    messageBytes.emplace_back(msgId % 0x100);
    messageBytes.emplace_back(msgId >> 8);

    for(const auto &el: payload)
    {
        messageBytes.emplace_back(el);
    }
}

Message::~Message()
{

}

Packet Message::toPacket()
{
    return Packet(messageBytes);
}

} // namespace Xerxes

#endif // !__MESSAGE_HPP