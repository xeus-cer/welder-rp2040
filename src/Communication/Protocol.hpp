#ifndef __PROTOCOL_HPP
#define __PROTOCOL_HPP

#include "RS485.hpp"
#include "Message.hpp"

namespace Xerxes
{
    

/**
 * @brief Protocol class
 * 
 * This class is used to implement the protocol.
 */
class Protocol
{
private:
    /**
     * @brief Pointer to the network interface
     * 
     */
    Network *xn;
public:
    Protocol(Network *network);
    ~Protocol();

    /**
     * @brief Send a message over the network
     * 
     * @param message message to send
     * @return true if the message was sent successfully
     * @return false if the message was not sent successfully
     */
    bool sendMessage(Message &message) const;

    /**
     * @brief Read a message from the network interface
     * 
     * @param message message to read into
     * @param timeoutUs timeout in microseconds
     * @return true if a message was read successfully
     * @return false if a message was not read successfully
     */
    bool readMessage(Message &message, const uint64_t timeoutUs);
};


} // namespace Xerxes


#endif // !__PROTOCOL_HPP