#include "Sensors/Generic/DigitalInputOutput.hpp"
#include "Hardware/Board/xerxes_rp2040.h"

#include "hardware/gpio.h"
#include <bitset>


namespace Xerxes
{


DigitalInputOutput::~DigitalInputOutput()
{
    this->stop();
}


void DigitalInputOutput::init(uint32_t iomask, uint32_t direction)
{
    // initialize GPIOs appearing in mask to use them as GPIOs
    gpio_init_mask(iomask);

    // Set GPIOs in mask to direction (input or output)
    gpio_set_dir_masked(iomask, direction);
    this->used_iomask = iomask;
    
    // TODO: Set pull-down resistors on inputs
}


void DigitalInputOutput::update()
{
    // Set GPIOs to whatever the values are in the pointer dv0
    gpio_put_masked(this->used_iomask, *_reg->dv0);
    auto state = gpio_get_all();
    auto gpio_pins = state & used_iomask;
    *_reg->dv1 = gpio_pins;
}


void DigitalInputOutput::stop()
{
    // Set GPIOs to input
    gpio_set_dir_in_masked(SHIELD_MASK);
}


std::ostream& operator<<(std::ostream& os, const DigitalInputOutput& dt)
{
    os << "DO: " << std::bitset<32>(*dt._reg->dv0) << ", DI: " << std::bitset<32>(*dt._reg->dv1) << std::endl;
    return os;
}


}   // namespace Xerxes

