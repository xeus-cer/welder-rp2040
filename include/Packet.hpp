#ifndef __PACKET_HPP
#define __PACKET_HPP


#define SOH 0x01 // start of header ASCII char


namespace Xerxes
{
    

class Packet
{
private:
    uint8_t checksum;
    std::vector<uint8_t> data = std::vector<uint8_t> {};

public:
    Packet(std::vector<uint8_t> &message);
    ~Packet();
    size_t size() const;
    std::vector<uint8_t> getData() const;
    static Packet EmptyPacket();
    uint8_t at(const uint8_t pos) const;
};



Packet::Packet(std::vector<uint8_t> &message)
{
    checksum = 0;
    uint8_t msgLen = message.size() + 3;
    data.emplace_back(SOH); // start of packet
    data.emplace_back(msgLen); // message length
    checksum = SOH + msgLen;

    for(const auto &el:message)
    {
        data.emplace_back(el);
        checksum += el;
    }

    checksum = ~(checksum & 0xFF) + 1; // two's complement
    data.emplace_back(checksum);
}


Packet::~Packet()
{
}


size_t Packet::size() const
{
    return data.size();
}


std::vector<uint8_t> Packet::getData() const
{
    return this->data;
}


Packet Packet::EmptyPacket()
{
    std::vector<uint8_t> emptyVector {};
    return Packet(emptyVector);
}


uint8_t Packet::at(const uint8_t pos) const
{
    return data.at(pos);
}


} // namespace Xerxes

#endif // !__PACKET_HPP