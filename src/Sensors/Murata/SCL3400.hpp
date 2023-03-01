#ifndef __SCL3400_HPP
#define __SCL3400_HPP


#include "SCL3X00.hpp"
#include <cmath>
#include "Hardware/Board/xerxes_rp2040.h"
#include <sstream>


namespace Xerxes
{


class SCL3400 : public SCL3X00
{
private:
    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @return double  - angle in degrees 
     */
    double getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet);
     
    // sensor specific variables
    static constexpr uint16_t _sensitivityModeA = 32768; // LSB/g

    constexpr static uint32_t _usInS = 1000000;  // microseconds in a second
    constexpr static uint32_t _sensorFreqHz = 10;  // sensor update frequency in Hz
    constexpr static uint32_t _sensorUpdateRateUs = _usInS / _sensorFreqHz;  // sensor update rate in microseconds

public:
    using SCL3X00::SCL3X00;

    /**
     * @brief Initialize the sensor for the first time after power up
     * 
     * Set sensor to Mode A - 10Hz, -30/+30 deg
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

#endif // !__SCL3400_HPP