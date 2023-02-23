#ifndef __PACKET_HPP
#define __PACKET_HPP


#include <vector>
#include <cstdint>


namespace Xerxes
{


/// @brief Start of header ASCII char
constexpr uint8_t SOH = 0x01; // start of header ASCII char
    

/**
 * @brief Packet container class
 * 
 * This class is used to store the data of a packet. 
 * 
 * The packet format is as follows:
 * SOH | LEN | DATA | CHECKSUM
 */
class Packet
{
private:
    /// @brief Checksum of the packet
    uint8_t checksum;

    /// @brief Data of the packet
    std::vector<uint8_t> data = std::vector<uint8_t> {};

    /// @brief Size of the packet
    size_t _size;
public:
    /**
     * @brief Construct a new Packet object
     * 
     */
    Packet();

    /**
     * @brief Construct a new Packet object from a vector
     * 
     * @param message vector to construct the packet from
     */
    Packet(std::vector<uint8_t> &message);
    ~Packet();

    /**
     * @brief Get the size of the packet
     * 
     * @return size_t size of the packet
     */
    size_t size() const;

    /**
     * @brief Get the Data object as a vector
     * 
     * @return std::vector<uint8_t> data of the packet
     */
    std::vector<uint8_t> getData() const;

    /**
     * @brief Get the empty packet
     * 
     * @return Packet empty packet
     */
    static Packet EmptyPacket();

    /**
     * @brief Get the data at a specific position
     * 
     * @param pos position of the data
     * @return uint8_t data at the position
     */
    uint8_t at(const uint8_t pos) const;
};


} // namespace Xerxes

#endif // !__PACKET_HPP