#include "Message.hpp"

#include "MessageId.h"


namespace Xerxes
{


Message::Message()
{
}


Message::Message(const Packet &packet)
{
    srcAddr = packet.at(2);
    dstAddr = packet.at(3);
    msgid_u msgIdRaw;
    msgIdRaw.msgid_8.msgid_l = packet.at(4);
    msgIdRaw.msgid_8.msgid_h = packet.at(5);
    msgId = msgIdRaw.msgid_16;

    for(uint16_t i=2; i<packet.size()-1; i++)
    {
        messageBytes.emplace_back(packet.at(i));
    }
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


std::vector<uint8_t>::const_iterator Message::payloadBegin() const
{
    return messageBytes.cbegin() + 4;
}


std::vector<uint8_t>::const_iterator Message::end() const
{
    return messageBytes.cend();
}


size_t Message::size() const
{
    return messageBytes.size();
}



uint8_t Message::at(const uint8_t pos) const
{
    return messageBytes.at(pos);
}


} // namespace Xerxes
