#ifndef __ACTIONS_HPP


#include <functional>
#include "UserFlash.hpp"
#include "DeviceIds.h"
#include "Sleep.hpp"


extern Xerxes::Slave xs;
extern Xerxes::Sensor *pSensor;


void measurementLoop()
{       
    // measure process value
    pSensor->update();
    pSensor->read(processValues);

    // for each process value and ring buffer
    for(int i=0; i<4; i++)
    {
        float pv = *processValues[i];
        ringBuffers[i].insertOne(pv);
        ringBuffers[i].updateStatistics();
        // update min, max stddev etc...
        ringBuffers[i].getStatistics(
            minimumValues[i],
            maximumValues[i],
            meanValues[i],
            standardDeviations[i]
        );
    }

}


/**
 * @brief Decorate a function to be unicast targeted
 * a function is called only on targeted unicast packet
 * @tparam Func 
 * @param f function to call for unicast packets
 * @return std::function<void(const Xerxes::Message &)> decorated lambda function
 */
template <typename Func>
std::function<void(const Xerxes::Message &)> unicast(Func f) 
{
  // The returned function is a lambda function that takes the same arguments as the original
  // function and calls it with the given arguments.
    return [f](const Xerxes::Message &msg) {
        if(msg.dstAddr!= 0xff && *devAddress == msg.dstAddr)
        {
        // Call the original function with the given arguments.
            f(msg);
        }
        else
        {       
            // do nothing
        }
    };
}


/**
 * @brief Decorate a function to be broadcast targeted
 * a function is called on targeted unicast packet or broadcast packet
 * 
 * @tparam Func 
 * @param f function to call for unicast packets
 * @return std::function<void(const Xerxes::Message &)> decorated lambda function
 */
template <typename Func>
std::function<void(const Xerxes::Message &)> broadcast(Func f) 
{
    return [f](const Xerxes::Message &msg) {
        if(msg.dstAddr == 0xff || *devAddress == msg.dstAddr)
        {
        // Call the original function with the given arguments.
            f(msg);
        }
        else
        {       
            // do nothing
        }
    };
}
    
/* callback functions */

void pingCallback(const Xerxes::Message &msg)
{
    std::vector<uint8_t> payload {DEVID_PRESSURE_60MBAR, PROTOCOL_VERSION_MAJ, PROTOCOL_VERSION_MIN};
    xs.send(msg.srcAddr, MSGID_PING_REPLY, payload);
}


void syncCallback(const Xerxes::Message &msg)
{
    measurementLoop();
}


void writeRegCallback(const Xerxes::Message &msg)
{
    uint8_t offset = msg.at(4);

    for(uint16_t i = 5; i < msg.size(); i++)
    {
        // IMPROVEMENT: implement READ_ONLY MEMORY
        uint8_t byte = msg.at(i);
        mainRegister[offset + i - 5] = byte;
    }
    updateFlash();
    xs.send(msg.srcAddr, MSGID_ACK_OK);
}


void readRegCallback(const Xerxes::Message &msg)
{
    /* Read  up to <LEN> bytes from device register, starting at <REG_ID>
     * The request prototype is <MSGID_READ> <REG_ID> <LEN> */

    uint8_t offset = msg.at(4);
    uint8_t len = msg.at(5);

    std::vector<uint8_t> payload {};

    for(uint16_t i = offset; i < offset + len; i++)
    {
        payload.emplace_back(mainRegister[i]);
    }

    xs.send(msg.srcAddr, MSGID_READ_VALUE, payload);

}


void sleepCallback(const Xerxes::Message &msg)
{
    uint8_t raw_duration[4];
    
    for(uint8_t i = 0; i < 4; i++)
    {
        raw_duration[i] = msg.at(i + 4);
    }

    uint32_t *durationUs = (uint32_t *)raw_duration;
    
    printf("LP Sleep for %d us.\n", *durationUs);
    sleep_lp(*durationUs);
}


void softResetCallback(const Xerxes::Message &msg)
{
    watchdog_reboot(0,0,0);
}


#endif // !__ACTIONS_HPP