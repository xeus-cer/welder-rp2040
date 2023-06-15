#ifndef __TIER_HPP
#define __TIER_HPP

#include "4DI4DO.hpp"

namespace Xeus
{

constexpr uint32_t GRABBER_MOTOR_PIN_FWD = Xerxes::DO0_PIN;  // 1 = forward, 0 = stop
constexpr uint32_t GRABBER_MOTOR_PIN_REV = Xerxes::DO1_PIN;  // 1 = reverse, 0 = stop
constexpr uint32_t TIER_MOTOR_PIN_FWD = Xerxes::DO2_PIN;  // 1 = forward, 0 = stop
constexpr uint32_t TIER_MOTOR_PIN_REV = Xerxes::DO3_PIN;  // 1 = reverse, 0 = stop

constexpr uint32_t JAWS_CLOSED_PIN = Xerxes::DI0_PIN;  // 1 = open, 0 = closed

constexpr uint32_t TYING_DURATION_MS = 1000;  // wind wire  from coil around rebar for 1 second
constexpr uint32_t CUTTING_DURATION_MS = 1000;  // cut wire for 1 second

constexpr uint32_t GRABBING_DURATION_MS = 1000;  // grab rebar for 1 second
constexpr uint32_t RELEASING_DURATION_MS = 1000;  // release rebar for 1 second

class Tier : public Xerxes::_4DI4DO
{
private:
    /// @brief convenience typedef
    typedef Xerxes::_4DI4DO super;

    uint64_t* status; // _reg->status;

    uint64_t timestampUs;

protected:
    void grabberFwd();
    void grabberRev();
    void grabberStop();
    void tierFwd();
    void tierRev();
    void tierStop();
    bool timeElapsed(uint32_t durationUs);

public:
    using Xerxes::_4DI4DO::_4DI4DO;

    /**
     * @brief Initialize the DIO
     */
    void init();
    
    /**
     * @brief Update the DIO
     */
    void update();


}; // class _4DI4DO

} // namespace Xerxes

#endif // !__TIER_HPP