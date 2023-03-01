#ifndef __ABP_HPP
#define __ABP_HPP


#include "Sensors/Sensor.hpp"
#include <ostream>
#include <string>

namespace Xerxes
{

// sensor specific variables
constexpr float VALmin  = 1638.0;   // counts = 10% 2^14
constexpr float VALmax  = 14745.0;  // counts = 90% 2^14
constexpr float Pmin    = 0.0;      // mbar    
constexpr float Pmax    = 60.0;    // mbar, or: 611.8298 mm

class ABP : public Sensor
{
protected:
    // typedef Sensor as super class for easier access
    typedef Sensor super;

public:
    using Sensor::Sensor;
    void init();
    void update();
    void stop();

    std::string getJson();
};


} //namespace Xerxes

#endif // !__ABP_HPP