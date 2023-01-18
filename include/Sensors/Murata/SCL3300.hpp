#ifndef __SCL3300_HPP
#define __SCL3300_HPP


#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "Sensors/Sensor.hpp"
#include <memory>
#include <array>
#include "xerxes_rp2040.h"

// TODO: THIS IS NOT COMPLETE - DO NOT USE YET
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
constexpr uint32_t Read_ERR_FLAG1 = 0x01C0000E;
constexpr uint32_t Read_ERR_FLAG2 = 0x200000C1;
constexpr uint32_t Read_CMD = 0x340000DF;
constexpr uint32_t Change_to_mode_1 = 0xB400001F;
constexpr uint32_t Change_to_mode_2 = 0xB4000102;
constexpr uint32_t Change_to_mode_3 = 0xB4000225;
constexpr uint32_t Change_to_mode_4 = 0xB4000338;
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

// sensor specific variables
// constexpr float VALmin  = 1638.0;   // counts = 10% 2^14

class SCL3300 : public Sensor
{
private:
    /**
     * @brief Exchange a block of data with the sensor
     * 
     * @param block 
     * @return uint32_t 
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
     * @brief Convert data from packet to angle in degrees
     * 
     * @param packet - received data
     * @return angle in degrees
     */
    double getDegFromPacket(SclPacket_t *packet);


    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @return double  - angle in degrees 
     */
    double getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet);


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
    void init();
    void update();
    void read(std::array<float*, 4> pvs);
    void stop();
};


void SCL3300::init()
{    
    constexpr uint spi_freq = 2 * MHZ;
    // init spi with freq , return actual frequency
    uint baudrate = spi_init(spi0, spi_freq);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Set the GPIO pin mux to the SPI
    gpio_set_function(SPI0_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CLK_PIN, GPIO_FUNC_SPI);

    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);

    // enable sensor 3V3
    gpio_put(EXT_3V3_EN_PIN, true);

    // CS pin as GPIO
    gpio_init(SPI0_CSN_PIN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_OUT);


    // chip init sequence    
    volatile uint32_t received_packet;
    
    sleep_ms(1);
    ExchangeBlock(CMD::Switch_to_bank_0);
    sleep_ms(3);

    received_packet = ExchangeBlock(CMD::SW_Reset);
    sleep_ms(3);

    received_packet = ExchangeBlock(CMD::Change_to_mode_4);
    sleep_ms(1);
    received_packet = ExchangeBlock(CMD::Enable_ANGLE);
    sleep_ms(100);

    received_packet = ExchangeBlock(CMD::Read_Status_Summary);
    received_packet = ExchangeBlock(CMD::Read_Status_Summary);
    received_packet = ExchangeBlock(CMD::Read_Status_Summary);
    sleep_ms(1);
    received_packet = ExchangeBlock(CMD::Read_WHOAMI);
    received_packet = ExchangeBlock(CMD::Read_WHOAMI);


    // read out first sequence - usually rubbish anyway
    this->update();
}


void SCL3300::update()
{    
    // pv0 = (float)((((p_val-VALmin)*(Pmax-Pmin))/(VALmax-VALmin)) + Pmin); 
    // pv3 = (float)((t_val*200.0/2047.0)-50.0);     // calculate internal temperature
    // declare packet variables
    auto packetX = std::make_unique<SclPacket_t>();
    auto packetY = std::make_unique<SclPacket_t>();
    auto packetZ = std::make_unique<SclPacket_t>();
    auto packetT = std::make_unique<SclPacket_t>();
    // pointer above is assigned to wrong memory location - <irq set enabled>
    
    ExchangeBlock(CMD::Read_ANG_X);
    longToPacket(ExchangeBlock(CMD::Read_ANG_Y), packetX);
    longToPacket(ExchangeBlock(CMD::Read_ANG_Z), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packetZ);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), packetT);

    // convert data to angles
    pv0 = static_cast<float>(getDegFromPacket(packetX));
    pv1 = static_cast<float>(getDegFromPacket(packetY));
    pv2 = static_cast<float>(getDegFromPacket(packetZ));

    // extract temperature from packet and convert to degrees
    uint16_t raw_temp = (uint16_t)(packetT->DATA_H << 8) + packetT->DATA_L;
    pv3 = -273 + (static_cast<float>(raw_temp) / 18.9);
}


void SCL3300::read(std::array<float*, 4> pvs)
{
    *pvs[0] = pv0;
    *pvs[1] = pv1;
    *pvs[2] = pv2;
    *pvs[3] = pv3;
}


void SCL3300::stop()
{
    gpio_put(EXT_3V3_EN_PIN, false);
    spi_deinit(spi0);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_IN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_IN);

}


uint32_t SCL3300::ExchangeBlock(const uint32_t &block)
{
    uint32_t rcvd;
    // SPI_CS active low, read data
    gpio_put(SPI0_CSN_PIN, 0);

    // exchange data
    spi_write_read_blocking(spi0, (uint8_t*)&block, (uint8_t*)&rcvd, 4);

    // release CS
    gpio_put(SPI0_CSN_PIN, 1);

    // sleep for 10us, NOTE: is it necessary?
    sleep_us(10);
    
    return rcvd;
}


double SCL3300::getDegFromPacket(SclPacket_t *packet)
{
    uint16_t raw = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    // raw = raw ^ 0x8000;
    double degrees = (double)raw * 180 / (1 << 15);
    return 180 + degrees;
}


double SCL3300::getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet)
{
    uint16_t raw = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    // raw = raw ^ 0x8000;
    double degrees = (double)raw * 180 / (1 << 15);
    return 180 + degrees;
}


double SCL3300::SclReadTemp()
{
    auto packet = std::make_unique<SclPacket_t>();

    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);


    uint16_t raw_temp = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    double degrees = -273 + ((double)raw_temp / 18.9);
    return degrees;
}


scl_status_t SCL3300::SclStatus()
{
    auto status = std::make_unique<SclPacket_t>();
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    
    // RS: '01' - Normal operation, no flags
    return static_cast<scl_status_t>(status->RS);
}


void SCL3300::longToPacket(const uint32_t data, SclPacket_t *packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


void SCL3300::longToPacket(const uint32_t data, std::unique_ptr<SclPacket_t>& packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


} //namespace Xerxes

#endif // !__SCL3300_HPP