#ifndef __SCL3X00_HPP
#define __SCL3X00_HPP


#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "Sensors/Sensor.hpp"
#include <memory>
#include <array>
#include "xerxes_rp2040.h"


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


// Calculate CRC for 24 MSB's of the 32 bit dword
// (8 LSB's are the CRC field and are not included in CRC calculation)
uint8_t CalculateCRC(uint32_t Data)
{
    uint8_t BitIndex;
    uint8_t BitValue;
    uint8_t CRC;
    CRC = 0xFF;
    for (BitIndex = 31; BitIndex > 7; BitIndex--)
    {
        BitValue = (uint8_t)((Data >> BitIndex) & 0x01);
        CRC = CRC8(BitValue, CRC);
    }
    CRC = (uint8_t)~CRC;
    return CRC;
}


uint8_t CRC8(uint8_t BitValue, uint8_t CRC)
{
    uint8_t Temp;
    Temp = (uint8_t)(CRC & 0x80);
    if (BitValue == 0x01)
    {
        Temp ^= 0x80;
    }
    CRC <<= 1;
    if (Temp > 0)
    {
        CRC ^= 0x1D;
    }
    return CRC;
}


class SCL3X00 : public Sensor
{
protected:
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
};


uint32_t SCL3X00::ExchangeBlock(const uint32_t &block)
{
    uint32_t rcvd;
    // SPI_CS active low, read data
    gpio_put(SPI0_CSN_PIN, 0);
    uint32_t reordered = __builtin_bswap32(block);

    sleep_us(5);

    // exchange data
    spi_write_read_blocking(spi0, (uint8_t*)&reordered, (uint8_t*)&rcvd, 4);

    // release CS
    gpio_put(SPI0_CSN_PIN, 1);

    rcvd = __builtin_bswap32(rcvd);
    sleep_us(5);
    
    return rcvd;
}


void SCL3X00::stop()
{
    gpio_put(EXT_3V3_EN_PIN, false);
    spi_deinit(spi0);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_IN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_IN);

}


void SCL3X00::longToPacket(const uint32_t data, SclPacket_t *packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


void SCL3X00::longToPacket(const uint32_t data, std::unique_ptr<SclPacket_t>& packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


double SCL3X00::SclReadTemp()
{
    auto packet = std::make_unique<SclPacket_t>();

    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);


    uint16_t raw_temp = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    double degrees = -273 + ((double)raw_temp / 18.9);
    return degrees;
}


scl_status_t SCL3X00::SclStatus()
{
    auto status = std::make_unique<SclPacket_t>();
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    
    // RS: '01' - Normal operation, no flags
    return static_cast<scl_status_t>(status->RS);
}



} // namespace Xerxes


#endif // !__SC3X00_HPP
