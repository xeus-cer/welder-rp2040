#ifndef __MESSAGE_HPP
#define __MESSAGE_HPP

#include <vector>
#include <cstdint>
#include "Packet.hpp"

namespace Xerxes
{
    

/**
 * @brief Message container class
 * 
 */
class Message
{
private:
    /// @brief Payload of the message
    std::vector<uint8_t> messageBytes {};
public:
    /// @brief Source address of the message
    uint8_t srcAddr;
    /// @brief Destination address of the message
    uint8_t dstAddr;
    /// @brief Message id of the message
    uint16_t msgId;

    /**
     * @brief Construct a new Message object
     * 
     * @param source 
     * @param destination 
     * @param msgid 
     * @param payload 
     */
    Message(const uint8_t source, const uint8_t destination, const uint16_t msgid, const std::vector<uint8_t> &payload);

    /**
     * @brief Construct a new Message object
     * 
     * @param source 
     * @param destination 
     * @param msgid 
     */
    Message(const uint8_t source, const uint8_t destination, const uint16_t msgid);

    /**
     * @brief Construct a new Message object from xerxes packet
     * 
     * @param packet valid packet to construct the message from
     */
    Message(const Packet &packet);

    /**
     * @brief Construct a new Message object
     * 
     */
    Message();

    /**
     * @brief Destroy the Message object
     * 
     */
    ~Message();

    /**
     * @brief Update the message with new data
     * 
     * @param source 
     * @param destination 
     * @param msgid 
     */
    void update(const uint8_t source, const uint8_t destination, const uint16_t msgid);

    /// @brief get the size of the message
    /// @return size_t size of the message
    size_t size() const;

    /// @brief get the byte at the given position
    uint8_t at(const uint8_t pos) const;

    /**
     * @brief Convert the message to a valid packet
     * 
     * @return Packet 
     */
    Packet toPacket();

    /**
     * @brief Get the Payload Begin object
     * 
     * @return std::vector<uint8_t>::const_iterator Iterator to the beginning of the payload
     */
    std::vector<uint8_t>::const_iterator payloadBegin() const;

    /**
     * @brief Get the Payload End object
     * 
     * @return std::vector<uint8_t>::const_iterator Iterator to the end of the payload
     */
    std::vector<uint8_t>::const_iterator end() const;

};


} // namespace Xerxes

#endif // !__MESSAGE_HPP