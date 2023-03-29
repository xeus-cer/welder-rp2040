#include "Sensors/Generic/Enviro/LightSound.hpp"
#include "Utils/Log.h"

namespace Xerxes
{

void LightSound::init()
{
    _devid = DEVID_LIGHT_SOUND_POLLUTION;  // device id
    
    // init ADC
    adc_init();
    
    // set pins as input
    adc_gpio_init(ADC0_PIN);
    adc_gpio_init(ADC1_PIN);
    adc_gpio_init(ADC2_PIN);
    adc_gpio_init(ADC3_PIN);

    // set update rate
    *_reg->desiredCycleTimeUs = _updateRateUs;

    // enable power supply to sensor
    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);
    // enable power supply for sensor
    gpio_put(EXT_3V3_EN_PIN, 1);
}

void LightSound::update()
{
    this->readMic();

    this->readLight();

    super::update();
}

void LightSound::readLight()
{
    uint64_t light2_buf = 0;
    uint64_t light3_buf = 0;
    // Using PT17-21C/L41/TR8  (datasheet:
    // https://datasheet.lcsc.com/lcsc/1810010213_Everlight-Elec-PT17-21C-L41-TR8_C100090.pdf)
    

    // if debug level is high enough, start timer
    #if (_LOG_LEVEL >= 4)
    auto start = time_us_64();
    #endif

    // set channel for led 2
    adc_select_input(2);
    // read samples and average
    for(uint16_t i = 0; i < overSample; i++)
    {
        light2_buf += adc_read();
    }

    // set channel for led 3
    adc_select_input(3);
    for(uint16_t i = 0; i < overSample; i++)
    {
        light3_buf += adc_read();
    }

    #if (_LOG_LEVEL >= 4)
    auto end = time_us_64();
    xlog_debug("Light captured in " << (end - start) << " us");
    #endif // _LOG_LEVEL

    // right shift by oversampleBits to decimate oversampled bits
    xlog_debug("light2_buf: " << light2_buf);
    xlog_debug("light3_buf: " << light3_buf);
    light2_buf >>= oversampleBits;
    light3_buf >>= oversampleBits;
    
    constexpr double Vref = 3.3;  // reference voltage
    constexpr double Vce = 0.4;  // voltage drop across LED
    constexpr double Vpt = Vref - Vce;  // output voltage
    constexpr double Iceo = 0.1e-6;  // current through dark LED
    double milliAmpLight2 = (light2_buf * Vpt / 4096) - Iceo;
    double milliAmpLight3 = (light3_buf * Vpt / 4096) - Iceo;
    
    // 1mA corresponds to irradiance of 1mW/cm^2 according to datasheet
    *(_reg->pv2) = milliAmpLight2;
    *(_reg->pv3) = milliAmpLight3;
}


double LightSound::micV2dB(double Voltage)
{
    /**
     * @note using microphone Goertek S15OT421-005
     * @note Mic sensitivity is -42dBV/Pa, that is approx 7.94mV/P (1/(100*10^(1/10)))
     * @note P0 = 20uPa
     * dB SPL = 20 * log10 ( ( Vampl / mic's sensitivity ) / 20 uPa )
     */

    constexpr double micSensitivity = 7.94e-3;  // in V/Pa
    double pressure = Voltage / micSensitivity;
    double dB = 20 * log10(pressure / 20e-6);
    return dB;
}


void LightSound::readMic()
{

    constexpr uint16_t numSamples = 100;
    uint64_t mic0_buf[numSamples] = {0};
    uint64_t mic1_buf[numSamples] = {0};

    // set channel for mic
    adc_select_input(0);

    #if (_LOG_LEVEL >= 4)
    auto start = time_us_64();
    #endif

    // read samples and average
    for(uint16_t i = 0; i < numSamples; i++)
    {
        uint64_t intermediate = 0;
        for(int i = 0; i < overSample; i++)
        {
            intermediate += adc_read();
        }
        mic0_buf[i] = intermediate / overSample;
    }

    #if (_LOG_LEVEL >= 4)
    double period = (time_us_64() - start) / 1e6;  // total period in s
    double samplePeriod = period / numSamples;  // in s
    double minFreq = 1 / period;  // in Hz

    xlog_debug("Mic captured in " << period << "s");
    xlog_debug("Sampling frequency: " << int(1 / samplePeriod) << "Hz");
    xlog_debug("Minimal frequency: " << int(minFreq) << "Hz");
    #endif // _LOG_LEVEL

    adc_select_input(1);
    for(uint16_t i = 0; i < numSamples; i++)
    {
        uint64_t intermediate = 0;
        for(int i = 0; i < overSample; i++)
        {
            intermediate += adc_read();
        }
        mic1_buf[i] = intermediate / overSample;
    }

    // calculate standard deviation    
    double sumOfElements0 {0};
    double sumOfSquaredErrors0 {0};
    double mean0 {0};
    double stdDev0 {0};
    double min0 {static_cast<double>(mic0_buf[0])};
    double max0 {static_cast<double>(mic0_buf[0])};
    double sumOfElements1 {0};
    double sumOfSquaredErrors1 {0};
    double mean1 {0};
    double stdDev1 {0};
    double min1 {static_cast<double>(mic1_buf[0])};
    double max1 {static_cast<double>(mic1_buf[0])};
    
    #if (_LOG_LEVEL >= 4)
    start = time_us_64();
    #endif

    for(int i=0; i<numSamples; i++)
    {
        auto el0 = mic0_buf[i];
        auto el1 = mic1_buf[i];
        sumOfElements0 += el0;
        sumOfElements1 += el1;

        if(el0 < min0)
        {
            min0 = el0;
        }

        if(el1 < min1)
        {
            min1 = el1;
        }

        if(el0 > max0)
        {
            max0 = el0;
        }

        if(el1 > max1)
        {
            max1 = el1;
        }
    }

    // mean0 value of the elements in buffer
    mean0 = sumOfElements0 / numSamples;
    mean1 = sumOfElements1 / numSamples;

    // standard deviation is calculated as the square root of the mean0 of the
    // squared deviations from the mean0
    for(int i=0; i<numSamples; i++)
    {
        sumOfSquaredErrors0 += powf(mic0_buf[i] - mean0, 2);
        sumOfSquaredErrors1 += powf(mic1_buf[i] - mean1, 2);
    }
    stdDev0 = sqrtf(sumOfSquaredErrors0 / numSamples);
    stdDev1 = sqrtf(sumOfSquaredErrors1 / numSamples);

    #if (_LOG_LEVEL >= 4)
    xlog_debug("Calculated in " << (time_us_64() - start) / 1e3f << "ms");
    std::stringstream ss;
    ss << "[";
    for(uint16_t i = 0; i < 10; i++)
    {
        ss << mic0_buf[i] << ", ";
    }
    ss << "... ";
    for(uint16_t i = numSamples - 10; i < numSamples; i++)
    {
        ss << ", " << mic0_buf[i];
    }

    xlog_debug("mic0_buf: " << ss.str() << "]");
    xlog_debug("min0: " << min0 * 3.3f / 4096 << "V");
    xlog_debug("max0: " << max0 * 3.3f / 4096 << "V");
    xlog_debug("amplitude: " << (max0 - min0) * 3.3f / 4096 << "V");
    xlog_debug("mean0: " << mean0 * 3.3f / 4096 << "V");
    xlog_debug("stdDev0: " << stdDev0 * 3.3f / 4096 << "V");
    #endif // _LOG_LEVEL
    
    double Vmic = (max0 - min0) * 3.3 / 4096;

    *(_reg->pv0) = micV2dB(Vmic);

    Vmic = (max1 - min1) * 3.3 / 4096;
    *(_reg->pv1) = micV2dB(Vmic);
}


std::string LightSound::getJson()
{
    std::stringstream ss;
    ss << "\n\t{";
    ss << "\n\t\"mic0\":" << *(_reg->pv0) << "dB" << ",";
    ss << "\n\t\"mic1\":" << *(_reg->pv1) << "dB" << ",";
    ss << "\n\t\"light2\":" << *(_reg->pv2) << "mW/cm^2" << ",";
    ss << "\n\t\"light3\":" << *(_reg->pv3) << "mW/cm^2";
    ss << "\n\t}";
    return ss.str();
}


void LightSound::stop()
{
    // disable power supply for sensor
    gpio_put(EXT_3V3_EN_PIN, 0);
}

} // namespace Xerxes
