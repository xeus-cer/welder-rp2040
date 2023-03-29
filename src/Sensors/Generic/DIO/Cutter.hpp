#ifndef __CUTTER_HPP
#define __CUTTER_HPP

#include <string>
#include "Encoder.hpp"

namespace Xerxes
{

constexpr uint32_t MOTOR_PIN_FWD = DO0_PIN;  // 1 = forward, 0 = reverse
constexpr uint32_t MOTOR_PIN_RUN = DO1_PIN;  // 1 = run, 0 = stop
constexpr uint32_t MOTOR_PIN_CUT = DO2_PIN;  // 1 = cut, 0 = release

constexpr uint32_t DEFAULT_MOTOR_RAMP_UP = 1000;  // 1000 pulses
constexpr uint32_t DEFAULT_MOTOR_RAMP_DOWN = 1000;  // 1000 pulses

constexpr uint32_t DEFAULT_MOTOR_STOP_TIME = 1'000'000;  // 1s
constexpr uint32_t DEFAULT_MOTOR_CUT_TIME = 1'000'000;  // 1s

class Cutter: public Encoder
{
private:
    /// @brief convenience typedef
    typedef Encoder super;
    uint32_t* pLength;  // = _reg->dv0;
    uint32_t* pOffset;  // = _reg->dv1;

    uint32_t* rampUpPulses; // _reg->config_val0;
    uint32_t* rampDownPulses; // _reg->config_val1;
    uint64_t* status; // _reg->status;

    uint64_t timestampUs;
    
public:
    using Encoder::Encoder;

    void init();
    void update();
};

}

#endif // __CUTTER_HPP