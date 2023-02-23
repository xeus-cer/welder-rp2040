#ifndef __NETWORK_HPP
#define __NETWORK_HPP

#include "Packet.hpp"


namespace Xerxes
{

/// @brief Broadcast address for the network - use this address to send a message to all nodes
constexpr uint8_t BROADCAST_ADDR = 0xff;


/**
 * @brief Network interface class
 * 
 * This class is used to implement the network interface. 
 */
class Network
{
private:
    //
public:
    /**
     * @brief Construct a new Network object
     * 
     */
    Network(/* args */);
    ~Network();

    /**
     * @brief Send data to the network - overload this function to implement the network interface
     * 
     * @param toSend packet to send
     * @return true if the packet was sent successfully
     * @return false if the packet was not sent successfully
     */
    virtual bool sendData(const Packet & toSend) const = 0;

    /**
     * @brief Read data from the network - overload this function to implement the network interface
     * 
     * @param timeoutUs timeout in microseconds
     * @param packet packet to read the data to
     * @return true if the packet was read successfully
     * @return false if the packet was not read successfully
     */
    virtual bool readData(const uint64_t timeoutUs, Packet &packet) = 0;
};


} // namespace Xerxes


#endif // !__NETWORK_HPP
