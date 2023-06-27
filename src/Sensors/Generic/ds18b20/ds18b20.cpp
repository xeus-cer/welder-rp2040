#include "ds18b20.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/adc.h"
#include <string>
#include <sstream>
#include "ds18b20.h"


namespace Xerxes
{


void DS18B20::init()
{
    this->init(2);
}


void DS18B20::init(int _numChannels)
{
    _devid = DEVID_TEMP_DS18B20;  // device id
    numChannels = _numChannels;
    
    // set update rate
    *_reg->desiredCycleTimeUs = _updateRateUs;

    // enable power supply to sensor
    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);
    // enable sensor 3V3
    gpio_put(EXT_3V3_EN_PIN, true);
    sleep_us(1000); // wait for sensor to power up

    startAll(temp_channel_0);
    if (numChannels > 1)
    {
        startAll(temp_channel_1);
    }
    if(numChannels > 2)
    {
        startAll(temp_channel_2);
    }
    if(numChannels > 3)
    {
        startAll(temp_channel_3);
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
    ss << "  \"Min\":" << getJsonMin() << "," << endl;
    ss << "  \"Max\":" << getJsonMax() << "," << endl;
    ss << "  \"Mean\":" << getJsonMean() << "," << endl;
    ss << "  \"StdDev\":" << getJsonStdDev() << endl;
    
    ss << "}";

    return ss.str();
}


}   // namespace Xerxes