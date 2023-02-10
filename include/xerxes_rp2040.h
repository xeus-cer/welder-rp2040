#ifndef XERXES_RP2040_H
#define XERXES_RP2040_H


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define USR_SW_PIN          18
#define USR_BTN_PIN         24
#define USR_LED_PIN         25
    
#define UART1_TX_PIN        8
#define UART1_RX_PIN        9
    
#define UART0_TX_PIN        16
#define UART0_RX_PIN        17
    
#define SPI0_MISO_PIN       0   
#define SPI0_CSN_PIN        1
#define SPI0_CLK_PIN        2
#define SPI0_MOSI_PIN       3
    
#define I2C0_SDA_PIN        4
#define I2C0_SCL_PIN        5
    
#define EXT_3V3_EN_PIN      6
    
#define RS_EN_PIN           19
#define RS_TX_PIN           UART0_TX_PIN
#define RS_RX_PIN           UART0_RX_PIN

#define ADC0_PIN            26  // ADC0 and PWM5_A share the same pin
#define ADC1_PIN            27  // ADC1 and PWM5_B share the same pin
#define ADC2_PIN            28  // ADC2 and PWM6_A share the same pin
#define ADC3_PIN            29  // ADC3 and PWM6_B share the same pin

#define TMP0_PIN            22  // TMP0 and I2C1 share the same pin
#define TMP1_PIN            23  // TMP1 and I2C1 share the same pin

#define I2C1_SDA_PIN        22  // I2C1 and TMP0 share the same pin
#define I2C1_SCL_PIN        23  // I2C1 and TMP1 share the same pin

#define PWM0_A_PIN          0   // PWM0_A and SPI0_MISO share the same pin
#define PWM0_B_PIN          1   // PWM0_B and SPI0_CSN share the same pin
#define PWM1_A_PIN          2   // PWM1_A and SPI0_CLK share the same pin
#define PWM1_B_PIN          3   // PWM1_B and SPI0_MOSI share the same pin
#define PWM2_A_PIN          4   // PWM2_A and I2C0_SDA share the same pin
#define PWM2_B_PIN          5   // PWM2_B and I2C0_SCL share the same pin
#define PWM3_A_PIN          22  // PWM3_A and TMP0 share the same pin
#define PWM3_B_PIN          23  // PWM3_B and TMP1 share the same pin
#define PWM4_A_PIN          24  // PWM4_A and USR_BTN share the same pin
#define PWM4_B_PIN          25  // PWM4_B and USR_LED share the same pin
#define PWM5_A_PIN          26  // PWM5_A and ADC0 share the same pin
#define PWM5_B_PIN          27  // PWM5_B and ADC1 share the same pin
#define PWM6_A_PIN          28  // PWM6_A and ADC2 share the same pin
#define PWM6_B_PIN          29  // PWM6_B and ADC3 share the same pin

#define CLK_GPIN1_PIN       22  // CLK_GPIN1 and TMP0 share the same pin
#define CLK_GPOUT1_PIN      23  // CLK_GPOUT1 and TMP1 share the same pin


// --- FLASH ---

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#undef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2

#undef PICO_XOSC_STARTUP_DELAY_MULTIPLIER
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64

#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

// All boards have B1 RP2040
#ifndef PICO_RP2040_B0_SUPPORTED 
#define PICO_RP2040_B0_SUPPORTED  0
#endif


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // XERXES_RP2040_H