#ifndef __PACKET_HPP
#define __PACKET_HPP

namespace Xerxes
{
    
class Packet
{
private:
    // neviem
public:
    const uint8_t soh {1};
    uint8_t msgLen;
    std::vector<uint8_t> data;
    uint8_t checksum;

    Packet(std::vector<uint8_t> &message);
    ~Packet();
    size_t size();
};

Packet::Packet(std::vector<uint8_t> &message)
{
    msgLen = message.size() + 2;
    checksum = soh + msgLen;
    data = message;

    for(const auto &el:data)
    {
        checksum += el;
    }
}

Packet::~Packet()
{
}

size_t Packet::size()
{
    return data.size() + 3;
}


} // namespace Xerxes

#endif // !__PACKET_HPP