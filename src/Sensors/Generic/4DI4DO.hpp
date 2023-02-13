#ifndef __4DI4DO_HPP
#define __4DI4DO_HPP

#include "Sensors/Generic/DigitalInputOutput.hpp"

namespace Xerxes
{


/// @brief GPIO pin masks GPIO0-5 | GPIO8-9
constexpr uint32_t _4IO_SHIELD_MASK = 0b1100111111;  // 0x33F


/// @brief DO pin masks
constexpr uint32_t DO0_DIR_MASK = 1 << 0; // DO0 = GPIO0
constexpr uint32_t DO1_DIR_MASK = 1 << 1; // DO1 = GPIO1
constexpr uint32_t DO2_DIR_MASK = 1 << 2; // DO2 = GPIO2
constexpr uint32_t DO3_DIR_MASK = 1 << 3; // DO3 = GPIO3

/// @brief DI pin masks
constexpr uint32_t DI0_MASK = 1 << 4; // DI0 = GPIO4
constexpr uint32_t DI1_MASK = 1 << 5; // DI1 = GPIO5
constexpr uint32_t DI2_MASK = 1 << 8; // DI2 = GPIO8
constexpr uint32_t DI3_MASK = 1 << 9; // DI3 = GPIO9

/// @brief DI pin direction masks
constexpr uint32_t DI0_DIR_MASK = 0 << 4; // DI0 = GPIO4
constexpr uint32_t DI1_DIR_MASK = 0 << 5; // DI1 = GPIO5
constexpr uint32_t DI2_DIR_MASK = 0 << 8; // DI2 = GPIO8
constexpr uint32_t DI3_DIR_MASK = 0 << 9; // DI3 = GPIO9

/// @brief DI shifts
constexpr uint32_t DI0_RSHIFT = 4;
constexpr uint32_t DI1_RSHIFT = 4;
constexpr uint32_t DI2_RSHIFT = 6;
constexpr uint32_t DI3_RSHIFT = 6;

/// @brief grouped pin masks
constexpr uint32_t DO_DIR_MASK = DO0_DIR_MASK | DO1_DIR_MASK | DO2_DIR_MASK | DO3_DIR_MASK;
constexpr uint32_t DI_DIR_MASK = DI0_DIR_MASK | DI1_DIR_MASK | DI2_DIR_MASK | DI3_DIR_MASK;

/// @brief all pin masks
constexpr uint32_t IO_DIR = DO_DIR_MASK | DI_DIR_MASK;


class _4DI4DO : public DigitalInputOutput
{
private:
    typedef DigitalInputOutput super;

public:
    using DigitalInputOutput::DigitalInputOutput;
    
    /**
     * @brief Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
     */
    void init();


    /**
     * @brief Update the DIO
     */
    void update();


}; // class _4DI4DO


} // namespace Xerxes


#endif // !__4DI4DO_HPP