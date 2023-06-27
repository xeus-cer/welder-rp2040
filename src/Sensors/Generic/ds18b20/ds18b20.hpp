#ifndef DS18B20_HPP
#define DS18B20_HPP

#include "Sensors/Sensor.hpp"

namespace Xerxes
{

class DS18B20: public Sensor
{
private:
    /// @brief convenience typedef
    typedef Sensor super;

    uint temp_channel_0 = 26;
    uint temp_channel_1 = 27;
    uint temp_channel_2 = 28;
    uint temp_channel_3 = 29;

    constexpr static uint32_t _updateRateHz = 10;  // update frequency in Hz
    constexpr static uint32_t _updateRateUs = _usInS / _updateRateHz;  // update rate in microseconds
    int numChannels;

public:
    using Sensor::Sensor;
    
    void init(int num_channels);
    void init();
    void update();
    void stop();
        
    /**
     * @brief Get the Json object representing the sensor values
     * 
     * @return std::string 
     */
    std::string getJson();

    std::string getJson(uint8_t channel);
    std::string getJsonLast();
    std::string getJsonMin();
    std::string getJsonMax();
    std::string getJsonMean();
    std::string getJsonStdDev();
};


} // namespace Xerxes

#endif // DS18B20_HPP