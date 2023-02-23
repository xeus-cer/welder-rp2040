#ifndef __SLAVE_HPP
#define __SLAVE_HPP

#include "Communication/Protocol.hpp"
#include <unordered_map>
#include <functional>
#include "Communication/MessageId.h"

namespace Xerxes
{


/**
 * @brief Slave class
 * 
 * It is used to bind the functions to the message ids and to call the
 * functions when a message with the corresponding message id is received.
 * 
 */
class Slave
{
private:
    Protocol *xp;
    std::unordered_map<msgid_t, std::function<void(const Message&)>> bindings;
    uint8_t address;

public:
    /**
     * @brief Construct a new Slave object
     * 
     */
    Slave();

    /**
     * @brief Construct a new Slave object
     * 
     * @param protocol pointer to the communication protocol
     * @param address address of the slave
     */
    Slave(Protocol *protocol, const uint8_t address);

    /**
     * @brief Destroy the Slave object
     * 
     */
    ~Slave();

    /**
     * @brief Bind a function to a message id
     * 
     * The function is called when a message with the corresponding message id is received.
     * 
     * @param msgId message id to bind the function to
     * @param _f function to bind to the message id. It is called when a message with the
     *  corresponding message id is received.
     */
    void bind(const msgid_t msgId, std::function<void(const Message&)> _f);

    /**
     * @brief Call the function bound to the message id
     * 
     * @param msg message to call the function with
     */
    void call(const Message &msg);

    /**
     * @brief Send a message
     * 
     * @param destinationAddress address of the destination
     * @param msgId message id of the message
     * @return true if the message was sent successfully
     * @return false if the message was not sent successfully
     */
    bool send(const uint8_t destinationAddress, const msgid_t msgId);

    /**
     * @brief Send a message
     * 
     * @param destinationAddress address of the destination
     * @param msgId message id of the message
     * @param payload payload of the message
     * @return true if the message was sent successfully
     * @return false if the message was not sent successfully
     */
    bool send(const uint8_t destinationAddress, const msgid_t msgId, const std::vector<uint8_t> &payload);

    /**
     * @brief Synchronize the slave with the master 
     * 
     * The slave is synchronized when it receives a valid message from the master. 
     * 
     * @param timeoutUs timeout in microseconds
     * @return true if the slave is synchronized
     * @return false if the slave is not synchronized
     */
    bool sync(const uint32_t timeoutUs);
};


} // namespace Xerxes

#endif // !__SLAVE_HPP