#include "Tier.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

namespace Xeus
{

void Tier::init()
{
    // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
    super::init();

    // bind status register for convenience
    status = _reg->status;
    *status = 0;

    _devid = DEVID_TIER;
}

void Tier::grabberFwd()
{
    gpio_put(GRABBER_MOTOR_PIN_FWD, 1);
    gpio_put(GRABBER_MOTOR_PIN_REV, 0);
}

void Tier::grabberRev()
{
    gpio_put(GRABBER_MOTOR_PIN_FWD, 0);
    gpio_put(GRABBER_MOTOR_PIN_REV, 1);
}

void Tier::grabberStop()
{
    gpio_put(GRABBER_MOTOR_PIN_FWD, 0);
    gpio_put(GRABBER_MOTOR_PIN_REV, 0);
}

void Tier::tierFwd()
{
    gpio_put(TIER_MOTOR_PIN_FWD, 1);
    gpio_put(TIER_MOTOR_PIN_REV, 0);
}

void Tier::tierRev()
{
    gpio_put(TIER_MOTOR_PIN_FWD, 0);
    gpio_put(TIER_MOTOR_PIN_REV, 1);
}

void Tier::tierStop()
{
    gpio_put(TIER_MOTOR_PIN_FWD, 0);
    gpio_put(TIER_MOTOR_PIN_REV, 0);
}

bool Tier::timeElapsed(uint32_t durationUs)
{
    return (time_us_64() - timestampUs) > durationUs;
}

void Tier::update()
{
    // the 
    if(*status == 0 && *_reg->dv0 != 0)
    { 
        // switch the tier on
        *status = 1;
        *_reg->dv0 = 0;
        grabberFwd();
        timestampUs = time_us_64();
    }

    if(*status == 1 && (timeElapsed(GRABBING_DURATION_MS * 1000)))
    {
        // switch the grabber off and the tier on
        *status = 2;
        grabberStop();
        tierFwd();
        timestampUs = time_us_64();
    }

    if(*status == 2 && timeElapsed(TYING_DURATION_MS * 1000))
    {
        // cut the wire
        *status = 3;
        tierRev();
        timestampUs = time_us_64();
    }

    if(*status == 3 && timeElapsed(CUTTING_DURATION_MS * 1000))
    {
        // switch the tier off and release the grabber
        *status = 4;
        tierStop();
        grabberRev();
        timestampUs = time_us_64();
    }

    if(*status == 4 && timeElapsed(RELEASING_DURATION_MS * 1000))
    {
        // switch the grabber off
        *status = 0;
        grabberStop();
    }
}

} // namespace Xeus
