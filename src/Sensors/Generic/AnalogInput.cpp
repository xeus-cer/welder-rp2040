#include "AnalogInput.hpp"

#include "Hardware/xerxes_rp2040.h"
#include "hardware/adc.h"


namespace Xerxes
{


AnalogInput::~AnalogInput()
{
    this->stop();
}


void AnalogInput::init()
{
    this->init(defaultChannels, defaultOversampleBits);
}


void AnalogInput::init(uint8_t numChannels, uint8_t oversampleBits)
{
    this->oversampleExtraBits = oversampleBits;
    this->numChannels = numChannels;
    this->overSample = 1 << (2 * oversampleBits);
    this->effectiveBitDepth = rpBitDepth + oversampleBits;
    this->numCounts = 1 << effectiveBitDepth;

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
        *pv0 = results[0] / static_cast<double>(numCounts);
    }
    else if(numChannels == 2)
    {
        *pv0 = results[0] / static_cast<double>(numCounts);
        *pv1 = results[1] / static_cast<double>(numCounts);
    }
    else if(numChannels == 3)
    {
        *pv0 = results[0] / static_cast<double>(numCounts);
        *pv1 = results[1] / static_cast<double>(numCounts);
        *pv2 = results[2] / static_cast<double>(numCounts);
    }
    else if(numChannels == 4)
    {
        *pv0 = results[0] / static_cast<double>(numCounts);
        *pv1 = results[1] / static_cast<double>(numCounts);
        *pv2 = results[2] / static_cast<double>(numCounts);
        *pv3 = results[3] / static_cast<double>(numCounts);
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


}   // namespace Xerxes