#ifndef __SCL3400_HPP
#define __SCL3400_HPP


#include "SCL3X00.hpp"
#include <cmath>
#include "Hardware/xerxes_rp2040.h"


namespace Xerxes
{


// sensor specific variables
// constexpr float VALmin  = 1638.0;   // counts = 10% 2^14

class SCL3400 : public SCL3X00
{
private:
    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @return double  - angle in degrees 
     */
    double getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet);

     
    // sensor specific variables
    static constexpr uint16_t sensitivityModeA = 32768; // LSB/g

public:
    void init();
    void update(bool calcStats);
};


} //namespace Xerxes

#endif // !__SCL3400_HPP