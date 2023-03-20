#include "Cutter.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

namespace Xerxes
{

void Cutter::init()
{
    // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
    super::init();

    // map memory to the registers
    pLength = _reg->dv0;
    pOffset = _reg->dv1;
    rampUpPulses = _reg->config_val0;
    rampDownPulses = _reg->config_val1;

    // Initialize the values of the registers to default values
    *pLength = 0;
    *pOffset = 0;
    *rampUpPulses = DEFAULT_MOTOR_RAMP_UP;
    *rampDownPulses = DEFAULT_MOTOR_RAMP_DOWN;

    _devid = DEVID_CUTTER;
}

void Cutter::update()
{
    // 1. rewind the bar if extruded length is too long
    
    
}


} // namespace Xerxes