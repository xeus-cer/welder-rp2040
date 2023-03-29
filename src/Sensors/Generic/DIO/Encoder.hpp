#ifndef __ENCODER_HPP
#define __ENCODER_HPP

#include <string>
#include "4DI4DO.hpp"

namespace Xerxes
{

constexpr uint32_t ENCODER_PIN_A = DI0_PIN;  // usually black
constexpr uint32_t ENCODER_PIN_B = DI1_PIN;  // usually white
constexpr uint32_t ENCODER_PIN_Z = DI2_PIN;  // usually orange/green

class Encoder : public _4DI4DO
{
private:
    /// @brief convenience typedef
    typedef _4DI4DO super;

protected:
    int32_t* encoderVal;

public:
    using _4DI4DO::_4DI4DO;
    
    void init();
    void update();
    void encoderIrqHandler(uint gpio);

    /**
     * @brief Get the Json object - returns sensor data as json string
     * 
     * @return std::string
     */
    std::string getJson();
};

}


#endif // __ENCODER_HPP