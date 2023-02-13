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
    
class Sensor : public Peripheral
{
protected:
    // typedef Peripheral as super class for easier access
    typedef Peripheral super;

    Register* _reg;

    // define ringbuffer (circular buffer) for each process value
    StatisticBuffer<float> rbpv0;
    StatisticBuffer<float> rbpv1;
    StatisticBuffer<float> rbpv2;
    StatisticBuffer<float> rbpv3;

public:
    using Peripheral::Peripheral;    
    Sensor(Register* reg);

    Sensor() : 
        _reg(nullptr)
    {};

    void update();
    
};


}   // namespace Xerxes

#endif // !__SENSOR_HPP
