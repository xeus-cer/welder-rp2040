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

protected:
    devid_t _devid{0};
    
public:
    Peripheral();
    ~Peripheral();
    
    // pure virtual functions
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void stop() = 0;

    devid_t getDevid();
};


}   // namespace Xerxes

#endif // !__PERIPHERAL_HPP