#ifndef __SENSOR_HPP
#define __SENSOR_HPP

#include "Core/Register.hpp"
#include "Sensors/Peripheral.hpp"
#include "Buffer/StatisticBuffer.hpp"
#include "Core/Definitions.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/adc.h"


namespace Xerxes
{


/**
 * @brief Sensor class
 * 
 */
class Sensor : public Peripheral
{
protected:
    // typedef Peripheral as super class for easier access
    typedef Peripheral super;

    Register* _reg;

    // define ringbuffer (circular buffer) for each process value
    /// @brief Ringbuffer for process value 0
    StatisticBuffer<float> rbpv0;
    /// @brief Ringbuffer for process value 1
    StatisticBuffer<float> rbpv1;
    /// @brief Ringbuffer for process value 2
    StatisticBuffer<float> rbpv2;
    /// @brief Ringbuffer for process value 3
    StatisticBuffer<float> rbpv3;

public:
    using Peripheral::Peripheral;    

    /**
     * @brief Construct a new Sensor object
     * 
     * @param reg pointer to the register where the sensor data is stored
     */
    Sensor(Register* reg);


    /**
     * @brief Construct a new Sensor object without register for declaration
     * 
     */
    Sensor() : 
        _reg(nullptr)
    {};

    /**
     * @brief Update the sensor data
     */
    void update();

    /**
     * @brief Get the Info object
     * 
     * @return std::string 
     */
    std::string getInfoJson();
    
};


}   // namespace Xerxes

#endif // !__SENSOR_HPP
