#ifndef __ABP_HPP
#define __ABP_HPP


#include "Sensors/Sensor.hpp"
#include <ostream>
#include <string>

namespace Xerxes
{

// sensor specific variables
constexpr float VALmin  = 1638.0;   // counts = 10% 2^14
constexpr float VALmax  = 14745.0;  // counts = 90% 2^14
constexpr float Pmin    = 0.0;      // Pa
constexpr float Pmax    = 6000.0;    // Pa

/**
 * @brief Honeywell ABP sensor
 * 
 * @details Density of used monopropylene glycol is -1,09010989[kg/m³/°C]*temp[°C] + 1045,824176[kg/m³]
 * 
 * +------------+------------+
 * | Temperature|   Density  |
 * |    (°C)    |  (kg/m³)   |
 * +------------+------------+
 * |    -20     |    1067    |
 * +------------+------------+
 * |    -15     |    1062    |
 * +------------+------------+
 * |    -10     |    1057    |
 * +------------+------------+
 * |    -5      |    1051    |
 * +------------+------------+
 * |     0      |    1046    |
 * +------------+------------+
 * |     5      |    1041    |
 * +------------+------------+
 * |     10     |    1035    |
 * +------------+------------+
 * |     15     |    1030    |
 * +------------+------------+
 * |     20     |    1024    |
 * +------------+------------+
 * |     25     |    1019    |
 * +------------+------------+
 * |     30     |    1013    |
 * +------------+------------+
 * |     35     |    1007    |
 * +------------+------------+
 * |     40     |    1002    |
 * +------------+------------+
 * 
 * 
 * @note 
 * 
 * @see 
 * 
 * @ingroup Sensors
 */

//TODO: temperature compensation for density of monopropylene glycol (MPG)

class ABP : public Sensor
{
protected:
    // typedef Sensor as super class for easier access
    typedef Sensor super;

public:
    using Sensor::Sensor;

    /**
     * @brief Init the sensor
     * 
     * Activate 3V3 power supply for the sensor, 
     * initialize SPI communication and read out first sequence
     */
    void init();

    /**
     * @brief Update the sensor
     * 
     * Read out sensor data and update register values in shared memory
     */
    void update();

    /**
     * @brief Stop the sensor
     * 
     * Disable sensor 3V3 therefore disabling the sensor
     */
    void stop();

    /**
     * @brief Get the Json object - returns sensor data as json string
     * 
     * @return std::string
     */
    std::string getJson();
};


} //namespace Xerxes

#endif // !__ABP_HPP