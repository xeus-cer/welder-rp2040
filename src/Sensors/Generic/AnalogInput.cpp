#include "AnalogInput.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
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
    _devid = DEVID_IO_4AI;  // device id
    
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
        *_reg->pv0 = results[0] / static_cast<double>(numCounts);
    }
    else if(numChannels == 2)
    {
        *_reg->pv0 = results[0] / static_cast<double>(numCounts);
        *_reg->pv1 = results[1] / static_cast<double>(numCounts);
    }
    else if(numChannels == 3)
    {
        *_reg->pv0 = results[0] / static_cast<double>(numCounts);
        *_reg->pv1 = results[1] / static_cast<double>(numCounts);
        *_reg->pv2 = results[2] / static_cast<double>(numCounts);
    }
    else if(numChannels == 4)
    {
        *_reg->pv0 = results[0] / static_cast<double>(numCounts);
        *_reg->pv1 = results[1] / static_cast<double>(numCounts);
        *_reg->pv2 = results[2] / static_cast<double>(numCounts);
        *_reg->pv3 = results[3] / static_cast<double>(numCounts);
    }
    else
    {
        // do nothing
    }


    // if calcStat is true, update statistics
    if(_reg->config->bits.calcStat)
    {
        // insert new values into ring buffer
        rbpv0.insertOne(*_reg->pv0);

        // update statistics
        rbpv0.updateStatistics();

        // update min, max stddev etc...
        rbpv0.getStatistics(_reg->minPv0, _reg->maxPv0, _reg->meanPv0, _reg->stdDevPv0);
    }

    // if calcStat is true and numChannels > 1, update statistics for pv1
    if(_reg->config->bits.calcStat && numChannels > 1)
    {
        rbpv1.insertOne(*_reg->pv1);
        rbpv1.updateStatistics();
        rbpv1.getStatistics(_reg->minPv1, _reg->maxPv1, _reg->meanPv1, _reg->stdDevPv1);
    }

    // if calcStat is true and numChannels > 2, update statistics for pv2
    if(_reg->config->bits.calcStat && numChannels > 2)
    {
        rbpv2.insertOne(*_reg->pv2);
        rbpv2.updateStatistics();
        rbpv2.getStatistics(_reg->minPv2, _reg->maxPv2, _reg->meanPv2, _reg->stdDevPv2);
    }

    // if calcStat is true and numChannels > 3, update statistics for pv3
    if(_reg->config->bits.calcStat && numChannels > 3)
    {
        rbpv3.insertOne(*_reg->pv3);
        rbpv3.updateStatistics();
        rbpv3.getStatistics(_reg->minPv3, _reg->maxPv3, _reg->meanPv3, _reg->stdDevPv3);
    }
}


void AnalogInput::stop()
{
    // nothing to do here
}


std::ostream& operator<<(std::ostream& os, const AnalogInput& ai)
{
    os << "AI0: " << *ai._reg->meanPv0 << ", AI1: " << *ai._reg->meanPv1 << ", AI2: " << *ai._reg->meanPv2 << ", AI3: " << *ai._reg->meanPv3 << std::endl;
    return os;
}


}   // namespace Xerxes