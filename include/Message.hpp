#ifndef __MESSAGE_HPP
#define __MESSAGE_HPP

#include <vector>
#include <cstdint>
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
    Message(const Packet &packet);
    Message();
    ~Message();
    void update(const uint8_t source, const uint8_t destination, const uint16_t msgid);
    size_t size() const;
    uint8_t at(const uint8_t pos) const;

    Packet toPacket();
    std::vector<uint8_t>::const_iterator payloadBegin() const;
    std::vector<uint8_t>::const_iterator end() const;

};


} // namespace Xerxes

#endif // !__MESSAGE_HPP