#ifndef __PERIPHERAL_HPP
#define __PERIPHERAL_HPP

#include <cstdint>
#include <xerxes-protocol/DeviceIds.h>
#include <ostream>

namespace Xerxes
{


/**
 * @brief Peripheral base class with pure virtual functions - init, update, stop
 * 
 */
class Peripheral
{
private:    

protected:
    devid_t _devid{0};
    
public:
    Peripheral();
    ~Peripheral();
    
    // pure virtual functions
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void stop() = 0;

    /**
     * @brief Get the Devid object
     * 
     * @return devid_t device id of the peripheral - based on the device type
     */
    devid_t getDevid();
};


}   // namespace Xerxes

#endif // !__PERIPHERAL_HPP