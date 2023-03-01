#ifndef __SCL3300A_HPP
#define __SCL3300A_HPP

#include "SCL3X00.hpp"

#include <complex>
#include <cmath>
#include <math.h>
#include <iterator>
#include <string>

namespace Xerxes
{

constexpr float G = 9.819f; // m/s^2
constexpr float SQRT2 = 1.41421356237f;
constexpr double PI = 3.1415926536;


class SCL3300a : public SCL3X00
{
private:
    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @param mode      - mode of sensor (acceleration or angle)
     * @return double  - angle in degrees 
     */
    double getAccFromPacket(const std::unique_ptr<SclPacket_t>& packet, const cmd_t mode = CMD::Change_to_mode_1);


    /**
     * @brief Initialize the sensor for the first time after power up
     * 
     * Set sensor to Mode A - 10Hz, -10/+10 deg
     */
    void initSequence();


    /// Flag to indicate if sensor needs to be initialized
    bool needInit = true;

public:
    using SCL3X00::SCL3X00;
    void init();
    void update();


    std::string getJson();
    std::string getJsonAmplitude();
    std::string getJsonLast();
    std::string getJsonMin();
    std::string getJsonMax();
    std::string getJsonMean();
    std::string getJsonStdDev(); 
};

} //namespace Xerxes    

#endif // !__SCL3300A_HPP