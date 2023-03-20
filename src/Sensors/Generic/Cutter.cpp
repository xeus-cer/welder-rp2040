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

    // bind status register for convenience
    status = _reg->status;
    *status = 0;

    _devid = DEVID_CUTTER;
}

void Cutter::update()
{
    // 1. rewind the bar if extruded length is too long
    if (*status == 1 && *encoderVal > *pLength + *rampUpPulses)
    {
        // rewind the bar
        gpio_put(MOTOR_PIN_FWD, 0);
        gpio_put(MOTOR_PIN_RUN, 1);
        *status = 1;
    }

    if (*status == 1 && *encoderVal <= *pLength + *rampUpPulses)
    {
        // the rod is retracted enough
        gpio_put(MOTOR_PIN_RUN, 0);
        timestampUs = time_us_64();
        *status = 2;
    }

    // 2. wait for motor to slow down
    if (*status == 2 && (time_us_64() - timestampUs > DEFAULT_MOTOR_STOP_TIME))
    {
        // the motor has probably stopped, start pushing the bar
        gpio_put(MOTOR_PIN_FWD, 1);
        gpio_put(MOTOR_PIN_RUN, 1);
        *status = 3;
    }    

    // 3. check if the bar is long enough, then cut
    if (*status == 3 && *encoderVal >= *pLength)
    {
        gpio_put(MOTOR_PIN_RUN, 0);
        gpio_put(MOTOR_PIN_CUT, 1);
        timestampUs = time_us_64();
        *status = 4;
    }

    // 4. wait for the bar to be cut
    if (*status == 4 && (time_us_64() - timestampUs > DEFAULT_MOTOR_CUT_TIME))
    {
        gpio_put(MOTOR_PIN_CUT, 0);
        *encoderVal = *pOffset;

        // push the bar to the place pOffset
        gpio_put(MOTOR_PIN_FWD, 1);
        gpio_put(MOTOR_PIN_RUN, 1);
        *status = 5;
    }

    // 5. wait for the bar to be pushed to the place pOffset - rampDownPulses
    //    and stop (let the motor slow down)
    if (*status == 5 && *encoderVal < *pOffset - *rampDownPulses)
    {
        gpio_put(MOTOR_PIN_RUN, 0);
        *status = 0;
    }
}


} // namespace Xerxes