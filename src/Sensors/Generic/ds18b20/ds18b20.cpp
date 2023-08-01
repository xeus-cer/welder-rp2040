#include "ds18b20.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/adc.h"
#include "Utils/Log.h"
#include <string>
#include <sstream>


namespace Xerxes
{


void DS18B20::init()
{
    this->init(3);
}


void DS18B20::init(int _numChannels)
{
    _devid = DEVID_TEMP_DS18B20;  // device id
    numChannels = _numChannels;
    xlog_info("Initializing DS18B20 using " << numChannels << " channels");
    
    // set update rate
    *_reg->desiredCycleTimeUs = _updateRateUs;

    rbpv0 = StatisticBuffer<float>(_updateRateHz * 5);
    rbpv1 = StatisticBuffer<float>(_updateRateHz * 5);
    rbpv2 = StatisticBuffer<float>(_updateRateHz * 5);
    rbpv3 = StatisticBuffer<float>(_updateRateHz * 5);

    // enable power supply to sensor
    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);
    // enable sensor 3V3
    gpio_put(EXT_3V3_EN_PIN, true);
    sleep_us(1000); // wait for sensor to power up

    gpio_init(temp_channel_0);
    if (numChannels > 1)
    {
        gpio_init(temp_channel_1);
    }
    if(numChannels > 2)
    {
        gpio_init(temp_channel_2);
    }
    if(numChannels > 3)
    {   
        gpio_init(temp_channel_3);
    }

    // update sensor values
    this->update();
}


void DS18B20::update()
{    
    // convert to value on scale <0, 1)
    // optimization: use if-else instead of switch, since numChannels is known at compile time
    if(numChannels == 1)
    {
        *_reg->pv0 = readJustOneTemp(temp_channel_0);
    }
    else if(numChannels == 2)
    {
        *_reg->pv0 = readJustOneTemp(temp_channel_0);
        *_reg->pv1 = readJustOneTemp(temp_channel_1);
    }
    else if(numChannels == 3)
    { 
        *_reg->pv0 = readJustOneTemp(temp_channel_0);
        *_reg->pv1 = readJustOneTemp(temp_channel_1);
        *_reg->pv2 = readJustOneTemp(temp_channel_2);
    }
    else if(numChannels == 4)
    {
        *_reg->pv0 = readJustOneTemp(temp_channel_0);
        *_reg->pv1 = readJustOneTemp(temp_channel_1);
        *_reg->pv2 = readJustOneTemp(temp_channel_2);
        *_reg->pv3 = readJustOneTemp(temp_channel_3);
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
        rbpv0.getStatistics(_reg->minPv0, _reg->maxPv0, nullptr, _reg->stdDevPv0, _reg->meanPv0);
    }

    // if calcStat is true and numChannels > 1, update statistics for pv1
    if(_reg->config->bits.calcStat && numChannels > 1)
    {
        rbpv1.insertOne(*_reg->pv1);
        rbpv1.updateStatistics();
        rbpv1.getStatistics(_reg->minPv1, _reg->maxPv1, nullptr, _reg->stdDevPv1, _reg->meanPv1);
    }

    // if calcStat is true and numChannels > 2, update statistics for pv2
    if(_reg->config->bits.calcStat && numChannels > 2)
    {
        rbpv2.insertOne(*_reg->pv2);
        rbpv2.updateStatistics();
        rbpv2.getStatistics(_reg->minPv2, _reg->maxPv2, nullptr, _reg->stdDevPv2, _reg->meanPv2);
    }

    // if calcStat is true and numChannels > 3, update statistics for pv3
    if(_reg->config->bits.calcStat && numChannels > 3)
    {
        rbpv3.insertOne(*_reg->pv3);
        rbpv3.updateStatistics();
        rbpv3.getStatistics(_reg->minPv3, _reg->maxPv3, nullptr, _reg->stdDevPv3, _reg->meanPv3);
    }
}


void DS18B20::stop()
{
    // disable sensor 3V3
    gpio_put(EXT_3V3_EN_PIN, false);
}


std::string DS18B20::getJsonLast()
{
    using namespace std;
    stringstream ss;

    ss << endl << "  {" << endl;
    ss << "    \"T0\": " << *this->_reg->pv0 << "," << endl;
    ss << "    \"T1\": " << *this->_reg->pv1 << "," << endl;
    ss << "    \"T2\": " << *this->_reg->pv2 << "," << endl;
    ss << "    \"T3\": " << *this->_reg->pv3 << endl;    
    ss << "  }";

    return ss.str();
}


std::string DS18B20::getJsonMin()
{
    using namespace std;
    stringstream ss;

    ss << endl << "  {" << endl;
    ss << "    \"Min(T0)\": " << *this->_reg->minPv0 << "," << endl;
    ss << "    \"Min(T1)\": " << *this->_reg->minPv1 << "," << endl;
    ss << "    \"Min(T2)\": " << *this->_reg->minPv2 << "," << endl;
    ss << "    \"Min(T3)\": " << *this->_reg->minPv3 << endl;    
    ss << "  }";

    return ss.str();
}


std::string DS18B20::getJsonMax()
{
    using namespace std;
    stringstream ss;

    ss << endl << "  {" << endl;
    ss << "    \"Max(T0)\": " << *this->_reg->maxPv0 << "," << endl;
    ss << "    \"Max(T1)\": " << *this->_reg->maxPv1 << "," << endl;
    ss << "    \"Max(T2)\": " << *this->_reg->maxPv2 << "," << endl;
    ss << "    \"Max(T3)\": " << *this->_reg->maxPv3 << endl;    
    ss << "  }";

    return ss.str();
}


std::string DS18B20::getJsonMean()
{
    using namespace std;
    stringstream ss;

    ss << endl << "  {" << endl;
    ss << "    \"Mean(T0)\": " << *this->_reg->meanPv0 << "," << endl;
    ss << "    \"Mean(T1)\": " << *this->_reg->meanPv1 << "," << endl;
    ss << "    \"Mean(T2)\": " << *this->_reg->meanPv2 << "," << endl;
    ss << "    \"Mean(T3)\": " << *this->_reg->meanPv3 << endl;    
    ss << "  }";

    return ss.str();
}


std::string DS18B20::getJsonStdDev()
{
    using namespace std;
    stringstream ss;

    ss << endl << "  {" << endl;
    ss << "    \"StdDev(T0)\": " << *this->_reg->stdDevPv0 << "," << endl;
    ss << "    \"StdDev(T1)\": " << *this->_reg->stdDevPv1 << "," << endl;
    ss << "    \"StdDev(T2)\": " << *this->_reg->stdDevPv2 << "," << endl;
    ss << "    \"StdDev(T3)\": " << *this->_reg->stdDevPv3 << endl;
    ss << "  }";

    return ss.str();
}


std::string DS18B20::getJson()
{
    using namespace std;
    stringstream ss;

    ss << endl << "{" << endl;
    ss << "  \"Last\":" << getJsonLast() << "," << endl;
    ss << "  \"Mean\":" << getJsonMean() << "," << endl;
    /*
    ss << "  \"Min\":" << getJsonMin() << "," << endl;
    ss << "  \"Max\":" << getJsonMax() << "," << endl;
    ss << "  \"StdDev\":" << getJsonStdDev() << endl;
    */
    ss << "}";

    return ss.str();
}


void DS18B20::_setLow(uint pin)
{   
    // set pin as output = clear the tris bit
    gpio_set_dir(pin, GPIO_OUT);
    // Set pin Low
    gpio_put(pin, 0);
}


void DS18B20::_setHigh(uint pin)
{
    // set pin as output = clear the tris bit
    gpio_set_dir(pin, GPIO_OUT);
    // set pin high
    gpio_put(pin, 1);
}


void DS18B20::_release(uint pin)
{
    // set pin as input
    gpio_set_dir(pin, GPIO_IN);
    // set pullup resistor
    gpio_pull_up(pin);
} 


unsigned DS18B20::_getVal(uint pin)
{
    //read pin 
    _release(pin);
    return gpio_get(pin);
}


unsigned DS18B20::_readBit(uint pin)
{
    _setLow(pin);
    sleep_us(5);
    _release(pin);
    sleep_us(5);
    unsigned sample = _getVal(pin);
    sleep_us(55);
    return sample;
}


void DS18B20::_write0(uint pin)
{
    _setLow(pin);
    sleep_us(65);
    _release(pin);
    sleep_us(10);            
}


void DS18B20::_write1(uint pin)
{
    _setLow(pin);
    sleep_us(5);
    _release(pin);
    sleep_us(60);          
}


void DS18B20::_writeBit(uint pin, unsigned data)
{
    if(data)
    {
        _write1(pin);
    }
    else
    {
        _write0(pin);
    }
}


char DS18B20::OWReadByte(uint pin)
{
    unsigned char loop;
    unsigned char result = 0;
    for (loop = 0; loop < 8; loop++)
    {
        result >>= 1; // shift the result to get it ready for the next bit to receive
        if (_readBit(pin))
        {
            result |= 0x80; // if result is one, then set MS-bit
        }
    }
    return (result);
}


void DS18B20::OWWriteByte(uint pin, char write_data)
{
    unsigned char loop;
    for (loop = 0; loop < 8; loop++)
    {
        _writeBit(pin, (write_data & 0x01)); //Sending LS-bit first
        write_data >>= 1; // shift the data byte for the next bit to send
    }
}


bool DS18B20::OWReset(uint pin)
{
    unsigned present = 0;
    _setLow(pin);
    sleep_us(500);
    _release(pin);
    sleep_us(60);
    
    // after reset, read bus
    present = _getVal(pin);
    
    sleep_us(440);
    
    //if device is present, return 1
    return !present;    
}


bool DS18B20::_writeAll(uint pin, char write_data)
{
    // reset bus takes approx 1ms
    bool present = OWReset(pin);
    // skip ROM

    // write 0xCC command to skip ROM - takes approx 70*8us = 560us
    OWWriteByte(pin, 0xCC);
    // read Temp, takes approx 750us
    OWWriteByte(pin, write_data);
    xlog_debug("WriteAll pin " << pin << " present " << present);
    return present;
}


bool DS18B20::startAll(uint pin)
{
    // write 0x44 command to start measurement of all sensors on the line
    return _writeAll(pin, 0x44);
}


double DS18B20::readJustOneTemp(uint pin)
{
    // write to all sensors 0xBE, must be just one connected.
    _writeAll(pin, 0xBE);
    char tempL = OWReadByte(pin);
    char tempH = OWReadByte(pin);
    // log values as decimal
    // convert low and high bytes to int16
    int16_t raw_temp = (tempH << 8) | tempL;
    double temp_c = 0.0625 * raw_temp;
    xlog_info("raw_temp: " << raw_temp << ", C°: " << temp_c);
    // convert to double and return
    startAll(pin); // retrig conversion

    if(temp_c > 125.0)
    {
        xlog_warn("Temperature is too high, sensor is probably disconnected.");
        return 0;
    }
    if(temp_c < -55.0)
    {
        xlog_warn("Temperature is too low, sensor is probably disconnected.");
        return 0;
    }

    return temp_c;
}


}   // namespace Xerxes