#include "Sensors/Peripheral.hpp"
#include <sstream>

namespace Xerxes
{

    bool isSpiDataOk(uint8_t *data, uint8_t len)
    {
        // check if data is ok. All bits must not be 0 or 1
        uint8_t first = data[0];
        if (first != 0 && first != 0xFF)
        {
            return true;
        }

        // first byte is 0 or 0xFF, check if all bytes are the same
        for (uint8_t i = 0; i < len; i++)
        {
            if (data[i] != first)
            {
                return true;
            }
        }

        // all bytes are 0 or 0xFF, return false.
        return false;
    }

    Peripheral::Peripheral()
    {
    }

    Peripheral::~Peripheral()
    {
    }

    devid_t Peripheral::getDevid()
    {
        return _devid;
    }

} // namespace Xerxes
