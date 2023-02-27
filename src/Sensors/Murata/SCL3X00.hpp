#ifndef __SCL3X00_HPP
#define __SCL3X00_HPP

#include <cstdint>
#include "Sensors/Sensor.hpp"
#include <memory>
#include <ostream>
#include <xerxes-protocol/DeviceIds.h>

typedef uint32_t cmd_t;

namespace CMD
{

constexpr uint32_t Read_ACC_X = 0x040000F7;
constexpr uint32_t Read_ACC_Y = 0x080000FD;
constexpr uint32_t Read_ACC_Z = 0x0C0000FB;
constexpr uint32_t Read_STO = 0x100000E9;
constexpr uint32_t Enable_ANGLE = 0xB0001F6F;
constexpr uint32_t Read_ANG_X = 0x240000C7;
constexpr uint32_t Read_ANG_Y = 0x280000CD;
constexpr uint32_t Read_ANG_Z = 0x2C0000CB;
constexpr uint32_t Read_Temperature = 0x140000EF;
constexpr uint32_t Read_Status_Summary = 0x180000E5;
constexpr uint32_t Read_ERR_FLAG1 = 0x1C0000E3;
constexpr uint32_t Read_ERR_FLAG2 = 0x200000C1;
constexpr uint32_t Read_CMD = 0x340000DF;
// SCL3300 specific commands
constexpr uint32_t Change_to_mode_1 = 0xB400001F;
constexpr uint32_t Change_to_mode_2 = 0xB4000102;
constexpr uint32_t Change_to_mode_3 = 0xB4000225;
constexpr uint32_t Change_to_mode_4 = 0xB4000338;
// SCL3400 specific commands
constexpr uint32_t Change_to_mode_A = 0xB400001F;
constexpr uint32_t Change_to_mode_B = 0xB4000338;

constexpr uint32_t Set_power_down_mode = 0xB400046B;
constexpr uint32_t Wake_up_from_power_down_mode = 0xB400001F;
constexpr uint32_t SW_Reset = 0xB4002098;
constexpr uint32_t Read_WHOAMI = 0x40000091;
constexpr uint32_t Read_SERIAL1 = 0x640000A7;
constexpr uint32_t Read_SERIAL2 = 0x680000AD;
constexpr uint32_t Read_current_bank = 0x7C0000B3;
constexpr uint32_t Switch_to_bank_0 = 0xFC000073;
constexpr uint32_t Switch_to_bank_1 = 0xFC00016E;

} // namespace CMD


namespace Xerxes
{


uint8_t CRC8(uint8_t BitValue, uint8_t CRC);
uint8_t CalculateCRC(uint32_t Data);
    
    
// sensor specific reply struct
typedef struct 
{
        uint8_t RW         :1;
        uint8_t ADDR       :5;
        uint8_t RS         :2;
        uint8_t DATA_H     :8;
        uint8_t DATA_L     :8;
        uint8_t CRC        :8;
} SclPacket_t;

typedef enum
{
    STARTUP = 0, // startup in progress
    NORMAL = 1, // normal operation, no flags
    N_A = 2, // N/A
    ERROR = 3 // error
} scl_status_t;



class SCL3X00 : public Sensor
{
protected:
    typedef Sensor super;

    /**
     * @brief Exchange a block of data with the sensor
     * 
     * @param block  - 32bit block to be sent
     * @return uint32_t - 32bit block received
     */
    uint32_t ExchangeBlock(const uint32_t &block);


    /**
     * @brief Convert a SclPacket_t struct to a 32bit integer
     * 
     * @param data 4 bytes from SPI transmission
     * @param packet  which will be populated
     */
    void longToPacket(const uint32_t data, SclPacket_t *packet);


    /**
     * @brief Convert a SclPacket_t struct to a 32bit integer
     * 
     * @param data 4 bytes from SPI transmission
     * @param packet  which will be populated
     */
    void longToPacket(const uint32_t data, std::unique_ptr<SclPacket_t>& packet);


    /**
     * @brief  Read temperature from sensor
     * 
     * @return temperature in degrees
     */
    double SclReadTemp();


    /**
     * @brief  Get status of the sensor
     * 
     * @return scl_status enum
     */
    scl_status_t SclStatus();


public:
    using Sensor::Sensor;
    void stop();

    // friend operator<< for easy printing
    friend std::ostream& operator<<(std::ostream& os, const SCL3X00& scl);
};


} // namespace Xerxes


#endif // !__SC3X00_HPP
