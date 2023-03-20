#ifndef __ENCODER_HPP
#define __ENCODER_HPP

#include <string>
#include "4DI4DO.hpp"

namespace Xerxes
{

constexpr uint32_t ENCODER_PIN_A = DI0_PIN;  // usually black
constexpr uint32_t ENCODER_PIN_B = DI1_PIN;  // usually white
constexpr uint32_t ENCODER_PIN_Z = DI2_PIN;  // usually orange/green

constexpr uint32_t MOTOR_PIN_FWD = DO0_PIN;  // 1 = forward, 0 = reverse
constexpr uint32_t MOTOR_PIN_RUN = DO1_PIN;  // 1 = run, 0 = stop
constexpr uint32_t MOTOR_PIN_CUT = DO2_PIN;  // 1 = cut, 0 = release


class Encoder : public _4DI4DO
{
private:
    /// @brief convenience typedef
    typedef _4DI4DO super;
    int intCtr = 0;
    bool lastPinA = false;

public:
    using _4DI4DO::_4DI4DO;
    Encoder() : _4DI4DO() {}
    ~Encoder() {}
    
    void init();
    void update();
    void encoderIrqHandler(uint gpio);
    void encoderZHandler(uint);

    /**
     * @brief Get the Json object - returns sensor data as json string
     * 
     * @return std::string
     */
    std::string getJson();
};

}


#endif // __ENCODER_HPP