#include "Sensors/Generic/DIO/DigitalInputOutput.hpp"
#include "Hardware/Board/xerxes_rp2040.h"

#include "hardware/gpio.h"
#include <bitset>
#include <sstream>

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

        // never use calcStat in this sensor
        _reg->config->bits.calcStat = 0;

        // Set the update rate
        *_reg->desiredCycleTimeUs = _updateRateUs;
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

    std::string DigitalInputOutput::getJson()
    {
        using namespace std;
        stringstream ss;

        ss << "{" << endl;
        ss << "\t\"DO\": " << *_reg->dv0 << "," << endl;
        ss << "\t\"DI\": " << *_reg->dv1 << endl;
        ss << "}" << endl;

        return ss.str();
    }

    std::string DigitalInputOutput::getInfoJson() const
    {
        uint64_t uuid = *_reg->uid;
        std::stringstream ss;
        ss << "{" << std::endl;
        ss << "  \"Address\": " << (int)*_reg->devAddress << "," << std::endl;
        ss << "  \"ID\": " << (int)_devid << "," << std::endl;
        // ss << "Type: " << typeid(this).name() << "," << std::endl;  // needs to enable rrti in cmake
        ss << "  \"Label\": \"" << _label << "\"," << std::endl;
        ss << "  \"UUID\": \"" << uuid << "\"," << std::endl;
        ss << "  \"Version\": \"" << __VERSION << "\"," << std::endl;
        ss << "  \"Build date\": \"" << __DATE__ << "\" " << std::endl;
        // ss << "  \"Errors\": 0b" << std::bitset<32>(*_reg->error) << "," << std::endl;
        ss << "  \"Status\": " << (int)*_reg->status << "," << std::endl;
        // ss << "  \"Update rate\": " << (int)*_reg->desiredCycleTimeUs << "," << std::endl;
        ss << "}" << std::endl;

        return ss.str();
    }

} // namespace Xerxes
