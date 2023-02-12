#ifndef __PACKET_HPP
#define __PACKET_HPP


#include <vector>
#include <cstdint>


namespace Xerxes
{


constexpr uint8_t SOH = 0x01; // start of header ASCII char
    

class Packet
{
private:
    uint8_t checksum;
    std::vector<uint8_t> data = std::vector<uint8_t> {};
    size_t _size;
public:
    Packet();
    Packet(std::vector<uint8_t> &message);
    ~Packet();
    size_t size() const;
    std::vector<uint8_t> getData() const;
    static Packet EmptyPacket();
    uint8_t at(const uint8_t pos) const;
};


} // namespace Xerxes

#endif // !__PACKET_HPP