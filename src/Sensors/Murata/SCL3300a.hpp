#ifndef __SCL3300A_HPP
#define __SCL3300A_HPP

#include "SCL3X00.hpp"

namespace Xerxes
{

constexpr float G = 9.819f; // m/s^2

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

public:
    using SCL3X00::SCL3X00;
    void init();
    void update();


    /**
     * @brief Get the AccelX, AccelY, AccelZ in
     * 
     * @param os output stream
     * @param scl sensor
     * @return std::ostream& 
     */
    friend std::ostream& operator<<(std::ostream& os, const SCL3300a& scl);
};

} //namespace Xerxes    

#endif // !__SCL3300A_HPP