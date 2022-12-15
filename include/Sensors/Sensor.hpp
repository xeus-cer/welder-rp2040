#ifndef __SENSOR_HPP
#define __SENSOR_HPP

namespace Xerxes
{
    
class Sensor
{
private:
    /* data */
public:
    Sensor(/* args */);
    ~Sensor();
    
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void read(std::array<float*, 4> pvs) = 0;
    virtual void stop() = 0;
};

Sensor::Sensor(/* args */)
{
}

Sensor::~Sensor()
{
}


}   // namespace Xerxes

#endif // !__SENSOR_HPP
