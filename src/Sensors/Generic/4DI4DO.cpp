#include "Sensors/Generic/4DI4DO.hpp"

namespace Xerxes
{
    
void _4DI4DO::init()
{
    // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
    super::init(_4IO_SHIELD_MASK, IO_DIR);
}


void _4DI4DO::update()
{
    // Update the DIO
    super::update();

    // Shift the DI values to the right place since shield is wired on GPIOs: 4,5,8,9
    bool di0 = (*_reg->dv1 & DI0_MASK) >> DI0_RSHIFT;
    bool di1 = (*_reg->dv1 & DI1_MASK) >> DI1_RSHIFT;
    bool di2 = (*_reg->dv1 & DI2_MASK) >> DI2_RSHIFT;
    bool di3 = (*_reg->dv1 & DI3_MASK) >> DI3_RSHIFT;

    // set the *dv1 pointer to the DI values shifted to the right place
    *_reg->dv1 = di0 | (di1 << 1) | (di2 << 2) | (di3 << 3);
}

} // namespace Xerxes
