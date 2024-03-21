#include "Welder.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Utils/Log.h"

namespace Xerxes
{

    WeldControl::WeldControl()
    {
        // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
        gpio_init(GRIP_PIN);
        gpio_init(RELEASE_PIN);
        gpio_init(WELD_PIN);

        gpio_set_dir(GRIP_PIN, GPIO_OUT);
        gpio_set_dir(RELEASE_PIN, GPIO_OUT);
        gpio_set_dir(WELD_PIN, GPIO_OUT);

        gpio_put(GRIP_PIN, 0);
        gpio_put(RELEASE_PIN, 0);
        gpio_put(WELD_PIN, 0);
    }

    void WeldControl::grip(uint32_t ms)
    {
        if (gpio_get(RELEASE_PIN))
        {
            // turn off the release pin if it is on
            gpio_put(RELEASE_PIN, 0);
        }
        gpio_put(GRIP_PIN, 1);
        xlog_info("Welder gripper on");
        sleep_ms(ms);
    }

    void WeldControl::release(uint32_t ms)
    {
        if (gpio_get(GRIP_PIN))
        {
            // turn off the grip pin if it is on
            gpio_put(GRIP_PIN, 0);
        }
        gpio_put(RELEASE_PIN, 1);
        xlog_info("Welder gripper off");
        sleep_ms(ms);
    }

    void WeldControl::weld()
    {
        if (gpio_get(GRIP_PIN) && !gpio_get(RELEASE_PIN))
        {
            // the gripper is on and the release is off
            gpio_put(WELD_PIN, 1);
            xlog_info("Welder welding");
        }
    }

    void WeldControl::weldStop()
    {
        gpio_put(WELD_PIN, 0);
        xlog_info("Welder welding stopped");
    }

    void Welder::init()
    {
        static_assert(
            !std::is_default_constructible<WeldControl>::value,
            "WeldControl must be a singleton!");
        static_assert(
            !std::is_copy_constructible<WeldControl>::value,
            "WeldControl must be a singleton!");
        static_assert(
            !std::is_copy_assignable<WeldControl>::value,
            "WeldControl must be a singleton!");

        // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
        super::init();
        xlog_info("Welder init");

        // initialize the controller controller
        controller = WeldControl::getInstance();
        xlog_info("Welder controller initialized");
        controller->weldStop();
        controller->release(0);
        xlog_info("Welder gripper released");

        // map memory to the registers
        pWeldTimeMs = _reg->dv0;
        pWeldingFor = _reg->pv0;
        pGripperOnTimeMs = _reg->config_val0;
        pGripperOffTimeMs = _reg->config_val1;
        xlog_info("Welder registers mapped");

        // Initialize the values of the registers to default values
        *pWeldTimeMs = 0;
        *pGripperOnTimeMs = DEFAULT_GRIPPER_ON_TIME_MS;
        *pGripperOffTimeMs = DEFAULT_GRIPPER_OFF_TIME_MS;
        xlog_info("Welder registers initialized");

        // bind status register for convenience
        status = _reg->status;
        *status = 0;
        xlog_info("Welder status initialized");

        // setup update rate to 10Hz
        *_reg->desiredCycleTimeUs = 1000000;

        _devid = DEVID_WELDER;
        xlog_info("Welder devid initialized");

        timer.user_data = this;
    }

    void Welder::update()
    {
        xlog_info(this->_servo_time_us << " " << this->_servo_pulse_us << " " << this->increment);
        // print status info
        xlog_info("Welder status: " << *status);

        if (!gpio_get(USR_BTN_PIN) && *status == 0)
        {
            *pWeldTimeMs = DEFAULT_WELD_TIME_MS;
        }

        if (
            *status == 0 &&
            *pWeldTimeMs > 0)
        {
            // the welder is idle and the weld time is set
            // start the cutting process:

            *status = 1;
            gpio_put(USR_LED_PIN, 1);
            // start the cutting process by gripping and toggle status to 1
            *pWeldingFor = 0;                    // reset the welding time
            controller->grip(*pGripperOnTimeMs); // return after the gripper is on
        }

        if (*status == 1)
        {
            // the bar is gripped
            controller->weld(); // return immediately
            xlog_info("Adding repeating timer");
            try
            {
                bool timer_added = add_repeating_timer_us(-100, _timerCallback, this, &timer);
                if (!timer_added)
                {
                    xlog_error("Failed to add repeating timer");
                }
            }
            catch (...)
            {
                xlog_error("Exception in adding repeating timer");
            }
            xlog_info("Repeating timer added");
            timestampUs = time_us_64();
            *status = 2;
        }

        if (*status == 2)
        {
            // the bar is welded
            *pWeldingFor = (float)(time_us_64() - timestampUs) / 1000000.0f;
            if (time_us_64() - timestampUs > *pWeldTimeMs * 1000)
            {
                // the weld time has elapsed
                controller->weldStop(); // return immediately
                *status = 3;
            }
        }

        if (*status == 3)
        {
            // the bar was welded, release the gripper
            controller->release(*pGripperOffTimeMs); // return after the gripper is off
            *pWeldTimeMs = 0;                        // reset the weld time so that the next weld can start
            *status = 0;                             // reset the status to 0 so that the next weld can start
            gpio_put(USR_LED_PIN, 0);
            bool cancelled = cancel_repeating_timer(&timer);
            if (!cancelled)
            {
                xlog_error("Failed to cancel repeating timer");
            }
            else
            {
                xlog_info("Repeating timer cancelled");
                gpio_put(MOVE_PIN, 0);
                sleep_us(5000);
                gpio_put(MOVE_PIN, 1);
                sleep_us(1500);
                gpio_put(MOVE_PIN, 0);
            }
        }
    }

    bool _timerCallback(repeating_timer_t *rt)
    {
        Welder *instance = (Welder *)rt->user_data;
        instance->increment++;
        instance->_servo_time_us += 100;
        if (instance->_servo_time_us > 5000)
        {
            instance->_servo_time_us = 0;
            instance->_servo_pulse_us += 5;
        }

        if (instance->_servo_pulse_us > 500)
        {
            instance->_servo_pulse_us = 0;
        }

        if (instance->_servo_time_us > 1250 + instance->_servo_pulse_us)
        {
            gpio_put(MOVE_PIN, 0);
        }
        else
        {
            gpio_put(MOVE_PIN, 1);
        }

        return true;
    }

} // namespace Xerxes