#ifndef __PERIPHERAL_HPP
#define __PERIPHERAL_HPP

#include <cstdint>
#include "Core/DeviceIds.h"
#include <ostream>

namespace Xerxes
{

class Peripheral
{
private:    
    devid_t _devid;
public:
    Peripheral();
    ~Peripheral();
    
    // pure virtual functions
    virtual void init();
    virtual void update();
    virtual void stop();

    devid_t getDevid();
};


}   // namespace Xerxes

#endif // !__PERIPHERAL_HPP