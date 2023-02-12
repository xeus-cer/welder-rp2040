#ifndef __SCL3300_HPP
#define __SCL3300_HPP

#include "SCL3X00.hpp"

// WIP, TODO: remove this after testing

namespace Xerxes
{


class SCL3300 : public SCL3X00
{
private:

    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @return double  - angle in degrees 
     */
    double getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet);



public:
    void init();
    void update();
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

    ExchangeBlock(CMD::SW_Reset);
    sleep_ms(3);

    ExchangeBlock(CMD::Change_to_mode_4);
    sleep_ms(1);
    ExchangeBlock(CMD::Enable_ANGLE);
    sleep_ms(100);

    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);
    sleep_ms(1);
    ExchangeBlock(CMD::Read_WHOAMI);
    ExchangeBlock(CMD::Read_WHOAMI);


    // read out first sequence - usually rubbish anyway
    this->update();
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
    // longToPacket(ExchangeBlock(CMD::Read_ANG_Z), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), packetT);

    // convert data to angles
    pv0 = static_cast<float>(getDegFromPacket(packetX));
    pv1 = static_cast<float>(getDegFromPacket(packetY));
    // pv2 = static_cast<float>(getDegFromPacket(packetZ));

    // extract temperature from packet and convert to degrees
    uint16_t raw_temp = (uint16_t)(packetT->DATA_H << 8) + packetT->DATA_L;
    pv3 = -273 + (static_cast<float>(raw_temp) / 18.9);
}


double SCL3300::getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet)
{
    uint16_t raw = (uint16_t)(packet->DATA_H << 8) + packet->DATA_L;
    raw = raw ^ 0x8000;
    double degrees = (double)raw * 180 / (1 << 15);
    return degrees - 180;
}


} //namespace Xerxes

#endif // !__SCL3300_HPP