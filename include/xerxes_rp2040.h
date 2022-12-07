#ifndef XERXES_RP2040_H
#define XERXES_RP2040_H


#define USR_SW_PIN  18
#define USR_BTN_PIN 24
#define USR_LED_PIN 25

#define UART1_TX_PIN    8
#define UART1_RX_PIN    9

#define UART0_TX_PIN    16
#define UART0_RX_PIN    17

#define SPI0_MISO_PIN   0   
#define SPI0_CSN_PIN    1
#define SPI0_CLK_PIN    2
#define SPI0_MOSI_PIN   3

#define I2C0_SDA_PIN    4
#define I2C0_SCL_PIN    5

#define EXT_3V3_EN_PIN  6

#define RS_EN_PIN   19
#define RS_TX_PIN   UART0_TX_PIN
#define RS_RX_PIN   UART0_RX_PIN


#ifdef __cplusplus
extern "C"
{
#endif 

    //code goes here

#ifdef __cplusplus
}
#endif 

#endif // XERXES_RP2040_H