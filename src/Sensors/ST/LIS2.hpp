#ifndef __LIS2_HPP
#define __LIS2_HPP

#include "Sensors/Sensor.hpp"

/**
 * @brief Register mapping for LIS2DW12
 * 
 * @see https://www.st.com/resource/en/datasheet/lis2dw12.pdf
 * 
 * @note The LIS2DW12 is a 3-axis accelerometer with a digital I2C/SPI interface. 
 *
 * Name                Type(1) Hex Binary      Default  Comment
 *  
 * OUT_T_L             R       0D  00001101    00000000 Temp sensor output
 * OUT_T_H             R       0E  00001110    00000000
 * WHO_AM_I            R       0F  00001111    01000100 Who am I ID
 * CTRL1               R/W     20  00100000    00000000 Control registers
 * CTRL2               R/W     21  00100001    00000100
 * CTRL3               R/W     22  00100010    00000000
 * CTRL4_INT1_PAD_CTRL R/W     23  00100011    00000000
 * CTRL5_INT2_PAD_CTRL R/W     24  00100100    00000000
 * CTRL6               R/W     25  00100101    00000000
 * OUT_T               R       26  00100110    00000000 Temp sensor output
 * STATUS              R       27  00100111    00000000 Status data register
 * OUT_X_L             R       28  00101000    00000000 Output registers
 * OUT_X_H             R       29  00101001    00000000
 * OUT_Y_L             R       2A  00101010    00000000
 * OUT_Y_H             R       2B  00101011    00000000
 * OUT_Z_L             R       2C  00101100    00000000
 * OUT_Z_H             R       2D  00101101    00000000
 * FIFO_CTRL           R/W     2E  00101110    00000000 FIFO control register
 * FIFO_SAMPLES        R       2F  00101111    00000000 Unread samples stored in FIFO
 * TAP_THS_X           R/W     30  00110000    00000000
 * TAP_THS_Y           R/W     31  00110001    00000000 Tap thresholds
 * TAP_THS_Z           R/W     32  00110010    00000000
 * INT_DUR             R/W     33  00110011    00000000 Interrupt duration
 * WAKE_UP_THS         R/W     34  00110100    00000000 Tap/double-tap selection, inactivity enable, wakeup threshold
 * WAKE_UP_DUR         R/W     35  00110101    00000000 Wakeup duration
 * FREE_FALL           R/W     36  00110110    00000000 Free-fall configuration
 * STATUS_DUP          R       37  00110111    00000000 Status register
 * WAKE_UP_SRC         R       38  00111000    00000000 Wakeup source
 * TAP_SRC             R       39  00111001    00000000 Tap source
 * SIXD_SRC            R       3A  00111010    00000000 6D source
 * ALL_INT_SRC         R       3B  00111011    00000000
 * X_OFS_USR           R/W     3C  00111100    00000000
 * Y_OFS_USR           R/W     3D  00111110    00000000
 * Z_OFS_USR           R/W     3E  00000100    00000000
 * CTRL_REG7           R/W     3F  00000100    00000000
 * */

namespace Xerxes
{

class LIS2 : public Sensor
{
protected:
    // typedef Sensor as super class for easier access
    typedef Sensor super;

    void writeReg(uint8_t reg, uint8_t data);

public:
    using Sensor::Sensor;

    void init();
    void update();
    void stop();
    std::string getJson();
};

} // namespace Xerxes

#endif // __LIS2_HPP