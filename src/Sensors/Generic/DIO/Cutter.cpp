#include "Cutter.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

namespace Xerxes
{

    Controller::Controller()
    {
        // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
        gpio_init(MOTOR_PIN_RUN);
        gpio_init(MOTOR_PIN_FWD);
        gpio_init(MOTOR_PIN_REV);
        gpio_init(MOTOR_PIN_CUT);

        gpio_set_dir(MOTOR_PIN_RUN, GPIO_OUT);
        gpio_set_dir(MOTOR_PIN_FWD, GPIO_OUT);
        gpio_set_dir(MOTOR_PIN_REV, GPIO_OUT);
        gpio_set_dir(MOTOR_PIN_CUT, GPIO_OUT);

        gpio_put(MOTOR_PIN_RUN, 0);
        gpio_put(MOTOR_PIN_FWD, 0);
        gpio_put(MOTOR_PIN_REV, 0);
        gpio_put(MOTOR_PIN_CUT, 0);
    }

    void Controller::forward()
    {
        if (gpio_get(MOTOR_PIN_REV))
        {
            // wait for the motor to stop
            gpio_put(MOTOR_PIN_REV, 0);
            sleep_us(DEFAULT_MOTOR_STOP_TIME);
        }
        gpio_put(MOTOR_PIN_FWD, 1);
        sleep_us(DEFAULT_CONTACTOR_LATENCY_US);
        gpio_put(MOTOR_PIN_RUN, 1);
    }

    void Controller::reverse()
    {
        if (gpio_get(MOTOR_PIN_FWD))
        {
            // wait for the motor to stop
            gpio_put(MOTOR_PIN_FWD, 0);
            sleep_us(DEFAULT_MOTOR_STOP_TIME);
        }
        gpio_put(MOTOR_PIN_CUT, 0);
        gpio_put(MOTOR_PIN_REV, 1);
        sleep_us(DEFAULT_CONTACTOR_LATENCY_US);
        gpio_put(MOTOR_PIN_RUN, 1);
    }

    void Controller::stop()
    {
        gpio_put(MOTOR_PIN_CUT, 0);
        gpio_put(MOTOR_PIN_RUN, 0);
        gpio_put(MOTOR_PIN_FWD, 0);
        gpio_put(MOTOR_PIN_REV, 0);
    }

    void Controller::cut_start()
    {
        if (gpio_get(MOTOR_PIN_RUN) && gpio_get(MOTOR_PIN_FWD) && !gpio_get(MOTOR_PIN_REV))
        {
            gpio_put(MOTOR_PIN_CUT, 1);
            sleep_us(DEFAULT_MOTOR_CUT_TIME);
            gpio_put(MOTOR_PIN_CUT, 0);
        }
    }

    void Controller::cut_stop()
    {
        gpio_put(MOTOR_PIN_CUT, 0);
    }

    void Cutter::init()
    {
        static_assert(
            !std::is_default_constructible<Controller>::value,
            "Controller must be a singleton!");
        static_assert(
            !std::is_copy_constructible<Controller>::value,
            "Controller must be a singleton!");
        static_assert(
            !std::is_copy_assignable<Controller>::value,
            "Controller must be a singleton!");

        // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
        super::init();

        // initialize the motor controller
        motor = Controller::getInstance();

        // map memory to the registers
        pLength = _reg->dv0;
        pOffset = _reg->dv1;
        rampUpPulses = _reg->config_val0;
        rampDownPulses = _reg->config_val1;
        pulsesPerMeter = _reg->config_val2;

        // Initialize the values of the registers to default values
        *pLength = 0;
        *pOffset = 0;
        *rampUpPulses = DEFAULT_MOTOR_RAMP_UP;
        *rampDownPulses = DEFAULT_MOTOR_RAMP_DOWN;
        *pulsesPerMeter = DEFAULT_PULSES_PER_METER;

        // bind status register for convenience
        status = _reg->status;
        *status = 0;

        _devid = DEVID_CUTTER;
    }

    void Cutter::update()
    {
        if (
            *status == 0 &&
            (int32_t)*pLength > 0)
        {
            // the bar is not being cut, but the length is set
            // start the cutting process:

            // calculate the lengths of the bar in pulses from pLength, offset (in mm)
            lengthPulses = *pLength * *pulsesPerMeter / 1000;
            offsetPulses = *pOffset * *pulsesPerMeter / 1000;

            if ((int32_t)*encoderVal >= (lengthPulses - (int32_t)*rampUpPulses))
            {
                // start rewinding the bar if it is not retracted enough
                motor->reverse();
            }

            // start the cutting process by toggling status to 1
            *status = 1;
        }

        // 1. rewind the bar if extruded length is too long
        //    wait for the bar to be retracted enough then
        //    stop rewinding the bar if it is retracted enough and start clock
        if (*status == 1 && (int32_t)*encoderVal <= (lengthPulses - (int32_t)*rampUpPulses))
        {
            // the rod is retracted enough, stop rewinding
            motor->stop();
            timestampUs = time_us_64();
            *status = 2; // status 2 means the motor is slowing down to a stop from the rewind
        }

        // 2. wait for motor to slow down
        if (*status == 2 && (time_us_64() - timestampUs > DEFAULT_MOTOR_STOP_TIME))
        {
            // the motor has probably stopped, start pushing the bar
            motor->forward();
            *status = 3; // status 3 means the bar is being pushed forward
        }

        // 3. check if the bar is long enough, then cut
        if (*status == 3 && (int32_t)*encoderVal >= lengthPulses)
        {
            // the bar is long enough, cut it
            motor->cut_start();
            timestampUs = time_us_64();

            *encoderVal = 0; // reset the encoder value right after the cut
            *status = 4;     // status 4 means the bar is being cut
        }

        // 4. wait for the bar to be cut then stop cutting
        if (*status == 4 && (time_us_64() - timestampUs > DEFAULT_MOTOR_CUT_TIME))
        {
            // the bar is cut, stop cutting
            motor->cut_stop();

            *status = 5; // status 5 means the bar is being pushed to the place pOffset
        }

        // 5. wait for the bar to be pushed to the place pOffset - rampDownPulses
        //    and stop (let the motor slow down)
        if (*status == 5 && (int32_t)*encoderVal >= offsetPulses - (int32_t)*rampDownPulses)
        {
            // the bar is pushed to the place pOffset - rampDownPulses, stop
            // pushing
            motor->stop();

            // clear the registers for the next cut
            *pLength = 0;
            *pOffset = 0;

            lengthPulses = 0;
            offsetPulses = 0;

            *status = 0; // status 0 means the bar is in the place pOffset and cutter is ready for the next cut
        }
    }

} // namespace Xerxes