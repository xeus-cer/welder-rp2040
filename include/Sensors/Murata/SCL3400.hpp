#ifndef __SCL3400_HPP
#define __SCL3400_HPP


#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "Sensors/Sensor.hpp"
#include <memory>
#include <array>
#include "xerxes_rp2040.h"
#include <math.h>


namespace CMD
{
    constexpr uint32_t Read_ACC_X = 0x040000F7;
    constexpr uint32_t Read_ACC_Y = 0x080000FD;
    constexpr uint32_t Read_Temperature = 0x140000EF;
    constexpr uint32_t Read_Status_Summary = 0x180000E5;
    constexpr uint32_t Read_ERR_FLAG1 = 0x1C0000E3;
    constexpr uint32_t Read_ERR_FLAG2 = 0x200000C1;
    constexpr uint32_t Read_CMD = 0x340000DF;
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

// sensor specific variables
// constexpr float VALmin  = 1638.0;   // counts = 10% 2^14

class SCL3400 : public Sensor
{
private:
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
     
    // sensor specific variables
    static constexpr uint16_t sensitivityModeA = 32768; // LSB/g

public:
    using Sensor::Sensor;
    void init();
    void update();
    void read(std::array<float*, 4> pvs);
    void stop();
};


void SCL3400::init()
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
    sleep_ms(3);

    // Write SW Reset command
    ExchangeBlock(CMD::SW_Reset);
    sleep_ms(3);

    /** 
     * Optional: 
     * 
     * Set Measurement mode Mode A (default)
     * 30° / 0.5g full-scale
     * 10 Hz 1st order low pass filter
    */
    // longToPacket(ExchangeBlock(CMD::Change_to_mode_A), packet);
    // sleep_ms(100);
    
    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);

    auto status = std::make_unique<SclPacket_t>();
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);

    // TODO: find out why this is not working, should return RS = NORMAL, but returns RS = ERROR instead
    // assert(status->RS == NORMAL);

    // read out first sequence - usually rubbish anyway
    this->update();
}


void SCL3400::update()
{    
    // pv0 = (float)((((p_val-VALmin)*(Pmax-Pmin))/(VALmax-VALmin)) + Pmin); 
    // pv3 = (float)((t_val*200.0/2047.0)-50.0);     // calculate internal temperature
    // declare packet variables
    auto packetX = std::make_unique<SclPacket_t>();
    auto packetY = std::make_unique<SclPacket_t>();
    auto packetT = std::make_unique<SclPacket_t>();
    
    ExchangeBlock(CMD::Read_ACC_X);
    longToPacket(ExchangeBlock(CMD::Read_ACC_Y), packetX);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), packetT);

    // convert data to angles
    pv0 = static_cast<float>(getDegFromPacket(packetX));
    pv1 = static_cast<float>(getDegFromPacket(packetY));

    // extract temperature from packet and convert to degrees
    uint16_t raw_temp = (uint16_t)(packetT->DATA_H << 8) + packetT->DATA_L;
    pv3 = -273 + (static_cast<float>(raw_temp) / 18.9);
}


void SCL3400::read(std::array<float*, 4> pvs)
{
    *pvs[0] = pv0;
    *pvs[1] = pv1;
    *pvs[2] = pv2;
    *pvs[3] = pv3;
}


void SCL3400::stop()
{
    gpio_put(EXT_3V3_EN_PIN, false);
    spi_deinit(spi0);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_IN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_IN);

}


uint32_t SCL3400::ExchangeBlock(const uint32_t &block)
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


double SCL3400::getDegFromPacket(SclPacket_t *packet)
{
    // TODO: finish this
    return 0;
}


double SCL3400::getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet)
{
    union {
        unsigned char data[2];
        int16_t acc;
    } value;

    value.data[0] = packet->DATA_L;
    value.data[1] = packet->DATA_H;

    // calculate acceleration in g
    double acceleration = static_cast<double>(value.acc) / sensitivityModeA;
    
    // convert to degrees using arcsin
    double degrees = asin(acceleration) * 180.0 / M_PI;
    
    return degrees;
}


double SCL3400::SclReadTemp()
{
    auto packet = std::make_unique<SclPacket_t>();

    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packet);


    uint16_t raw_temp = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    double degrees = -273 + ((double)raw_temp / 18.9);
    return degrees;
}


scl_status_t SCL3400::SclStatus()
{
    auto status = std::make_unique<SclPacket_t>();
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);
    
    // RS: '01' - Normal operation, no flags
    return static_cast<scl_status_t>(status->RS);
}


void SCL3400::longToPacket(const uint32_t data, SclPacket_t *packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


void SCL3400::longToPacket(const uint32_t data, std::unique_ptr<SclPacket_t>& packet)
{
    packet->RW        = static_cast<uint8_t>(data >> 31);
    packet->ADDR      = static_cast<uint8_t>(data >> 26);
    packet->RS        = static_cast<uint8_t>(data >> 24);
    packet->DATA_H    = static_cast<uint8_t>(data >> 16);
    packet->DATA_L    = static_cast<uint8_t>(data >> 8);
    packet->CRC       = static_cast<uint8_t>(data);
}


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

} //namespace Xerxes

#endif // !__SCL3400_HPP