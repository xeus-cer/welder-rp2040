#ifndef __WELDER_HPP
#define __WELDER_HPP

#include <string>
#include "4DI4DO.hpp"
#include "pico/time.h"

namespace Xerxes
{

    constexpr uint32_t GRIP_PIN = DO0_PIN;    // 1 = forward, 0 = reverse
    constexpr uint32_t RELEASE_PIN = DO1_PIN; // 1 = run, 0 = stop
    constexpr uint32_t WELD_PIN = DO2_PIN;    // 1 = weld, 0 = stop
    constexpr uint32_t MOVE_PIN = DO3_PIN;    // 1 = move, 0 = stop

    constexpr uint32_t DEFAULT_GRIPPER_ON_TIME_MS = 1'000; // 1s
    constexpr uint32_t DEFAULT_GRIPPER_OFF_TIME_MS = 500;  // 0.5s

    class WeldControl
    {
    private:
        WeldControl();

    public:
        static WeldControl *getInstance()
        {
            static WeldControl *instance;
            if (!instance)
                instance = new WeldControl;
            return instance;
        }

        /**
         * @brief delete copy constructor and assignment operator
         *
         */
        WeldControl(WeldControl const &) = delete;

        /**
         * @brief delete copy constructor and assignment operator
         *
         */
        void operator=(WeldControl const &) = delete;

        /**
         * @brief
         *
         */
        void grip(uint32_t ms);
        void release(uint32_t ms);
        void weld();
        void weldStop();
    };

    bool _timerCallback(repeating_timer_t *rt);

    /**
     * @brief Welder class
     *
     * @details Welder class is a child of 4DI4DO class and inherits all of its
     * methods and variables. It adds the functionality of gripping and welding
     *
     * To weld a part:
     * 1. set pWeldTimeMs [_reg->dv0] to desired duration in ms
     * 2. poll status [_reg->status] until it cycles through 1-5 back to 0
     * 3. rinse and repeat from step 1 *
     */
    class Welder : public _4DI4DO
    {
    private:
        /// @brief convenience typedef
        typedef _4DI4DO super;
        uint32_t *pWeldTimeMs; // = _reg->dv0;

        uint32_t *pGripperOnTimeMs;  // _reg->config_val0;
        uint32_t *pGripperOffTimeMs; // _reg->config_val1;

        uint64_t *status;   // _reg->status;
        float *pWeldingFor; // _reg->pv0;

        WeldControl *controller;

        uint64_t timestampUs;

        int _servo_pulse_us = 0;
        int _servo_time_us = 0;
        uint64_t increment = 0;
        repeating_timer_t timer;

    protected:
        std::string _label{"XUES Welder peripheral"};

    public:
        using _4DI4DO::_4DI4DO;

        void init();

        /**
         * @brief update the status of the welder and gripper
         *
         * @details the gripper is controlled by DO0 and DO1 [optional] and the
         *
         */
        void update();

        friend bool _timerCallback(repeating_timer_t *rt);
    };

}

#endif // __WELDER_HPP