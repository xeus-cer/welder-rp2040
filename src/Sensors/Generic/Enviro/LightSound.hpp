#ifndef __LIGHT_SOUND_HPP
#define __LIGHT_SOUND_HPP

#include "Sensors/Sensor.hpp"
#include <string>

namespace Xerxes
{
    
class LightSound : public Sensor
{
private:
    typedef Sensor super;

    /// @brief read ADC channel for microphone (adc0 + adc1)
    void readMic();

    /// @brief read ADC channel for light sensor (adc2 + adc3)
    void readLight();

    /**
     * @brief convert voltage to dB
     * 
     * @param Voltage from analog input
     * @return double decibells
     */
    double micV2dB(double Voltage);

    // sensor specific variables                    
    constexpr static uint8_t rpBitDepth = 12;                   // bit depth of the ADC, 12          
    uint8_t oversampleBits          = 3;                        // extra bits for oversampling
    uint16_t overSample             = 1 << (2 * oversampleBits);  // oversampling factor, 4^oversampleBits
    uint8_t effectiveBitDepth       = rpBitDepth + oversampleBits;  // effective bit depth, 12 + 3 = 15
    uint64_t numCounts              = 1 << effectiveBitDepth;   // number of counts, 2^15 = 32768

    constexpr static uint32_t _updateRateHz = 20;  // update frequency in Hz
    constexpr static uint32_t _updateRateUs = _usInS / _updateRateHz;  // update rate in microseconds

protected:
    // nothing yet

public:
    using super::super;

    void init();
    void update();
    void stop();

    std::string getJson();
};


} // namespace Xerxes


#endif // __LIGHT_SOUND_HPP