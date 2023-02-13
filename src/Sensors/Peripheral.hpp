#ifndef __PERIPHERAL_HPP
#define __PERIPHERAL_HPP

#include <cstdint>

namespace Xerxes
{

class Peripheral
{
private:

public:
    Peripheral();
    ~Peripheral();
    
    // pure virtual functions
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void stop() = 0;
};

}   // namespace Xerxes

#endif // !__PERIPHERAL_HPP