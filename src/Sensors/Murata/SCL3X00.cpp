#include "SCL3X00.hpp"

#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "Hardware/Board/xerxes_rp2040.h"
#include "pico/time.h"


namespace Xerxes
{


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


std::ostream& operator<<(std::ostream& os, const SCL3X00& scl)
{
    os << "X: " << *scl._reg->meanPv0 << "°, Y: " << *scl._reg->meanPv1 << "°, Temp: " << *scl._reg->meanPv3 << "°C" << std::endl;
    return os;
}

}