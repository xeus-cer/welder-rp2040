#ifndef __SCL3300_HPP
#define __SCL3300_HPP

#include "SCL3X00.hpp"

namespace Xerxes
{


class SCL3300 : public SCL3X00
{
private:

    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @return double  - angle in degrees 
     */
    double getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet);

    /**
     * @brief Initialize the sensor for the first time after power up
     * 
     * Set sensor to Mode 4 - 10Hz, -10/+10 deg
     */
    void initSequence();

    /// Flag to indicate if sensor needs to be initialized
    bool needInit = true;

public:
    using SCL3X00::SCL3X00;

    /**
     * @brief Init the sensor
     * 
     */
    void init();

    /**
     * @brief Update the sensor
     * 
     * Read out sensor data and update register values in shared memory
     */
    void update();

    /**
     * @brief Get the Json object - returns sensor data as json string
     * 
     * @return std::string
     */
    std::string getJson();
};


} //namespace Xerxes

#endif // !__SCL3300_HPP