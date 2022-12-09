#ifndef __PACKET_HPP
#define __PACKET_HPP


#define SOH 0x01 // start of header ASCII char


namespace Xerxes
{
    

class Packet
{
private:
    uint8_t checksum;
    std::vector<uint8_t> data = std::vector<uint8_t>();
public:

    Packet(std::vector<uint8_t> &message);
    ~Packet();
    size_t size();
    const std::vector<uint8_t> &getData();
};


Packet::Packet(std::vector<uint8_t> &message)
{
    checksum = 0;
    uint8_t msgLen = message.size() + 2;
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


size_t Packet::size()
{
    return data.size() + 3;
}


const std::vector<uint8_t> &Packet::getData()
{
    return this->data;
}

} // namespace Xerxes

#endif // !__PACKET_HPP