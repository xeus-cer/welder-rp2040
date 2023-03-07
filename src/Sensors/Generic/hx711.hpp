#ifndef __HX711_HPP
#define __HX711_HPP

#include "Sensors/Sensor.hpp"

namespace Xerxes
{

/**
 * @brief HX711 ADC
 */ 
class HX711 : public Sensor
{
private:
    /// @brief convenience typedef
    typedef Sensor super;

    /// @brief read data pin
    bool data();

    /// @brief set clock pin to high/low
    void clock(bool level);

    constexpr static uint32_t _sensorFreqHz = 80;  // sensor update frequency in Hz
    constexpr static uint32_t _sensorUpdateRateUs = _usInS / _sensorFreqHz;  // sensor update rate in microseconds

public:
    using Sensor::Sensor;

    /**
     * @brief Initialize the HX711 ADC
     */
    void init();

    /**
     * @brief Read the HX711 ADC
     * @return uint32_t ADC value
     */
    int32_t read();
    
    void update();
    void stop();

    std::string getJson();

};

} // namespace Xerxes

#endif  // __HX711_HPP

