#ifndef __ANALOG_INPUT
#define __ANALOG_INPUT

#include "hardware/adc.h"
#include "Sensors/Sensor.hpp"
#include "Hardware/xerxes_rp2040.h"


namespace Xerxes
{

/// @brief default number of oversampling bits
constexpr uint8_t defaultOversampleBits     = 3;
/// @brief default number of used ADC channels
constexpr uint8_t defaultChannels           = 4;
/// @brief bit depth of RP2040 ADC
constexpr uint8_t rpBitDepth                = 12;

/**
 * @brief Analog input sensor class
 * 
 * @details AnalogInput is a generic class for up to 4 analog inputs, e.g. 4x 0-3.3V
 * 
 * @note default ADC depth on RP2040 is 12 bit, 8.7 ENOB (approx. 54dB SNR)
 * @note AnalogInput uses oversampling to increase resolution, increasing SNR by 6dB per bit
 * @note n-bit oversampling increases sampling time too: sample time = 4^n * conversion time 
 */
class AnalogInput : public Sensor
{
private:
    uint64_t results[4] = {0, 0, 0, 0};  // up to 4 channels, 64 bit to avoid overflow
    // sensor specific variables                              // oversampling bits
    uint8_t oversampleExtraBits     = defaultOversampleBits;   // extra bits for oversampling, 4
    uint16_t overSample             = 1 << (2 * defaultOversampleBits);   // oversampling factor, 4^4 = 256
    uint8_t effectiveBitDepth       = rpBitDepth + defaultOversampleBits;   // effective bit depth, 12 + 4 = 16
    uint64_t numCounts              = 1 << effectiveBitDepth;           // number of counts, 2^16 = 65536
    uint8_t numChannels             = 4;                                // number of channels, default is 4

public:
    /**
     * @brief Construct a new AnalogInput object
     * 
     * @param numChannels number of channels, e.g. 4
     * @param oversampleBits number of bits to oversample, e.g. 3 bits -> 64x oversampling
     * @note oversampleBits increase duration of update() by 4^oversampleBits, e.g. 3 bits -> 256 samples per channel per update
     * @note mean sampling time for 4 channels with no oversampling is 10us 
     * @note mean sampling time for 2 channels with 3 bits oversampling is 5us * 4^2 = 320us
     * @note mean sampling time for 1 channel with 5 bits oversampling is 2.5us * 4^5 = 2560us
     */
    AnalogInput(uint8_t numChannels, uint8_t oversampleBits = defaultOversampleBits) : 
        oversampleExtraBits(oversampleBits),
        numChannels(numChannels),
        overSample(1 << (2 * oversampleBits)), 
        effectiveBitDepth(rpBitDepth + oversampleBits), 
        numCounts(1 << effectiveBitDepth) {};
    

    /**
     * @brief Construct a new Analog Input object
     * 
     * @note default is 4 channels with 3 bits oversampling
     * 
     */
    AnalogInput() : 
        oversampleExtraBits(defaultOversampleBits),
        overSample(1 << (2 * defaultOversampleBits)), 
        effectiveBitDepth(rpBitDepth + defaultOversampleBits), 
        numCounts(1 << effectiveBitDepth),
        numChannels(defaultChannels) {};

    /**
     * @brief Destroy the 4AI object    
     * 
     */
    ~AnalogInput();

    /**
     * @brief init sensor, set pins as input, init ADC
     * 
     */
    void init();

    /**
     * @brief update sensor values, takes approx 1ms for 4 channels with 100 samples
     * 
     */
    void update();

    /**
     * @brief Not needed for 4xAI
     * 
     */
    void stop();
};


void AnalogInput::init()
{
    // init ADC
    adc_init();
    
    // set pins as input
    adc_gpio_init(ADC0_PIN);
    adc_gpio_init(ADC1_PIN);
    adc_gpio_init(ADC2_PIN);
    adc_gpio_init(ADC3_PIN);

    // update sensor values
    this->update();
}


AnalogInput::~AnalogInput()
{
    this->stop();
}


void AnalogInput::update()
{    
    // oversample and average over 4 channels, effectively increasing bit depth by 4 bits
    // https://www.silabs.com/documents/public/application-notes/an118.pdf
    for(uint8_t channel = 0; channel < numChannels; channel++)
    {
        // set channel
        adc_select_input(channel);
        results[channel] = 0;

        // read samples and average
        for(uint16_t i = 0; i < overSample; i++)
        {
            results[channel] += adc_read();
        }

        // right shift by oversampleExtraBits to decimate oversampled bits
        results[channel] >>= oversampleExtraBits;
    }
    
    // convert to value on scale <0, 1)
    // optimization: use if-else instead of switch, since numChannels is known at compile time
    if(numChannels == 1)
    {
        pv0 = results[0] / static_cast<double>(numCounts);
    }
    else if(numChannels == 2)
    {
        pv0 = results[0] / static_cast<double>(numCounts);
        pv1 = results[1] / static_cast<double>(numCounts);
    }
    else if(numChannels == 3)
    {
        pv0 = results[0] / static_cast<double>(numCounts);
        pv1 = results[1] / static_cast<double>(numCounts);
        pv2 = results[2] / static_cast<double>(numCounts);
    }
    else if(numChannels == 4)
    {
        pv0 = results[0] / static_cast<double>(numCounts);
        pv1 = results[1] / static_cast<double>(numCounts);
        pv2 = results[2] / static_cast<double>(numCounts);
        pv3 = results[3] / static_cast<double>(numCounts);
    }
    else
    {
        // do nothing
    }
}


void AnalogInput::stop()
{
    // nothing to do here
}

} // namespace Xerxes

#endif // !__ANALOG_INPUT