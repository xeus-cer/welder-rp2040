#ifndef __SENSOR_HPP
#define __SENSOR_HPP

#include <array>
#include "Sensors/Peripheral.hpp"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/adc.h"


namespace Xerxes
{
    
class Sensor : public Peripheral
{
protected:
    float* pv0;    
    float* pv1;
    float* pv2;
    float* pv3;
public:
    using Peripheral::Peripheral;    
    Sensor(float *pv0, float *pv1, float *pv2, float *pv3) :
        pv0(pv0), pv1(pv1), pv2(pv2), pv3(pv3)
    {};
    Sensor() :
        pv0(nullptr), pv1(nullptr), pv2(nullptr), pv3(nullptr)
    {};
    
};


}   // namespace Xerxes

#endif // !__SENSOR_HPP
