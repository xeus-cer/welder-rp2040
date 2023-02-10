#ifndef __SENSOR_HPP
#define __SENSOR_HPP

namespace Xerxes
{
    
class Sensor
{
protected:
    double pv0 = 0;    
    double pv1 = 0;
    double pv2 = 0;
    double pv3 = 0;
public:
    Sensor(/* args */);
    ~Sensor();
    
    // pure virtual functions
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void stop() = 0;
    
    void read(std::array<float*, 4> pvs);
};


void Sensor::read(std::array<float*, 4> pvs)
{
    *pvs[0] = pv0;
    *pvs[1] = pv1;
    *pvs[2] = pv2;
    *pvs[3] = pv3;
}


Sensor::Sensor(/* args */)
{
}


Sensor::~Sensor()
{
}


}   // namespace Xerxes

#endif // !__SENSOR_HPP
