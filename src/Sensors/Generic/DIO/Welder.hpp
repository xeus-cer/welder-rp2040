#ifndef __CUTTER_HPP
#define __CUTTER_HPP

#include <string>
#include "Encoder.hpp"

namespace Xerxes
{

    constexpr uint32_t GRIP_PIN = DO0_PIN; // 1 = forward, 0 = reverse
    constexpr uint32_t PUSH_PIN = DO1_PIN; // 1 = run, 0 = stop
    constexpr uint32_t WELD_PIN = DO2_PIN; // 1 = cut, 0 = release

    constexpr uint32_t DEFAULT_GRIPPER_TIME = 1'000'000; // 1s
    constexpr uint32_t DEFAULT_WELD_TIME = 3'000'000;    // 1s

    /**
     * @brief Cutter class
     *
     * @details Cutter class is a child of Encoder class and inherits all of its
     * methods and variables. It adds the functionality of cutting the bar to the
     * desired length and offset.
     *
     * To cut the bar, the following steps are taken:
     * 1. [optional] set pOffset [_reg->dv1] to desired offset in mm
     * 2. set pLength [_reg->dv0] to desired length in mm
     * 3. poll status [_reg->status] until it cycles through 1-5 back to 0
     * 4. rinse and repeat from step 1 *
     */
    class Cutter : public _4DI4DO
    {
    private:
        /// @brief convenience typedef
        typedef Encoder super;
        uint32_t *pLength; // = _reg->dv0;
        int32_t lengthPulses;
        uint32_t *pOffset; // = _reg->dv1;
        int32_t offsetPulses;

        uint32_t *rampUpPulses;   // _reg->config_val0;
        uint32_t *rampDownPulses; // _reg->config_val1;
        uint32_t *pulsesPerMeter; // _reg->config_val2;

        uint64_t *status; // _reg->status;

        uint64_t timestampUs;

    public:
        using Encoder::Encoder;

        void init();

        /**
         * @brief update the status of the cutter and handle the cutting process
         *
         * @details the cutting process is as follows:
         * 0. motor is either slowing down or stopped and waiting for the next cut
         * 1. rewind the bar if extruded length is too long
         * 2. wait for motor to slow down from rewind
         * 3. push the bar forward to the desired length
         * 4. cutting is happening
         * 5. cut is done and the bar is being pushed to the desired offset
         *
         */
        void update();
    };

}

#endif // __CUTTER_HPP