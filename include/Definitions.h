#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "hardware/flash.h"
#include "hardware/clocks.h"

#define DEFAULT_BAUDRATE    115200

#define NON_VOLATILE_SIZE   FLASH_PAGE_SIZE       // 256
#define REGISTER_SIZE       NON_VOLATILE_SIZE * 2

#define RX_TX_QUEUE_SIZE    256 // bytes
// #undef PICO_FLASH_SIZE_BYTES
#define FLASH_TARGET_OFFSET 2 * 1024 * 1024 - (FLASH_SECTOR_SIZE)

// how many samples are rotated in ring buffer
#define RING_BUFFER_LEN     100

/* memory mapping offsets: */
/* non volatile range */
#define GAIN_PV0_OFFSET     0
#define GAIN_PV1_OFFSET     4
#define GAIN_PV2_OFFSET     8
#define GAIN_PV3_OFFSET     12

#define OFFSET_PV0_OFFSET   16
#define OFFSET_PV1_OFFSET   20
#define OFFSET_PV2_OFFSET   24
#define OFFSET_PV3_OFFSET   28

#define OFFSET_CYCLE_TIME   32
#define OFFSET_ADDRESS      36

#define OFFSET_CONFIG_BITS  40
#define OFFSET_CLOCK_KHZ    44


//volatile range
#define STATUS_OFFSET       NON_VOLATILE_SIZE + 0       // 256
#define ERROR_OFFSET        NON_VOLATILE_SIZE + 8       // 264
#define UID_OFFSET          NON_VOLATILE_SIZE + 16      // 272

#define PV0_OFFSET          NON_VOLATILE_SIZE + 20      // 276
#define PV1_OFFSET          NON_VOLATILE_SIZE + 24      // 280
#define PV2_OFFSET          NON_VOLATILE_SIZE + 28      // 284
#define PV3_OFFSET          NON_VOLATILE_SIZE + 32      // 288

#define MEAN_PV0_OFFSET     NON_VOLATILE_SIZE + 36      // 292
#define MEAN_PV1_OFFSET     NON_VOLATILE_SIZE + 40      // 296
#define MEAN_PV2_OFFSET     NON_VOLATILE_SIZE + 44      // 300 
#define MEAN_PV3_OFFSET     NON_VOLATILE_SIZE + 48      // 304
 
#define STDEV_PV0_OFFSET    NON_VOLATILE_SIZE + 52      // 308
#define STDEV_PV1_OFFSET    NON_VOLATILE_SIZE + 56      // 312
#define STDEV_PV2_OFFSET    NON_VOLATILE_SIZE + 60      // 316
#define STDEV_PV3_OFFSET    NON_VOLATILE_SIZE + 64      // 320

#define MIN_PV0_OFFSET      NON_VOLATILE_SIZE + 68      // 324
#define MIN_PV1_OFFSET      NON_VOLATILE_SIZE + 72      // 328
#define MIN_PV2_OFFSET      NON_VOLATILE_SIZE + 76      // 332
#define MIN_PV3_OFFSET      NON_VOLATILE_SIZE + 80      // 336
 
#define MAX_PV0_OFFSET      NON_VOLATILE_SIZE + 84      // 340
#define MAX_PV1_OFFSET      NON_VOLATILE_SIZE + 88      // 344
#define MAX_PV2_OFFSET      NON_VOLATILE_SIZE + 92      // 348
#define MAX_PV3_OFFSET      NON_VOLATILE_SIZE + 96      // 352

/* config masks */
/* If true use free run, if false: wait for sync packet */
#define MASK_CONFIG_FREE_RUN        1<<0
/* If true, enable low power sleep and disable USB */
#define MASK_CONFIG_LOW_POWER       1<<1


/* Default values */
#define DEFAULT_CYCLE_TIME_US       100'000     // 100ms
#define DEFAULT_WATCHDOG_DELAY      100         // ms

/* Default clocks */
#define DEFAULT_USB_CLOCK_FREQ      48 * MHZ
#define DEFAULT_XOSC_CLOCK_FREQ     12 * MHZ

#define DEFAULT_ADC_CLOCK_SRC       CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC
#define DEFAULT_ADC_CLOCK_FREQ      DEFAULT_XOSC_CLOCK_FREQ
#define DEFAULT_PERI_CLOCK_SRC      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB
#define DEFAULT_PERI_CLOCK_FREQ     DEFAULT_USB_CLOCK_FREQ

/* system clock */
#define DEFAULT_SYS_CLOCK_SRC       CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS
#define DEFAULT_SYS_PLL_FREQ        1500 * MHZ
#define DEFAULT_SYS_PLL_POST_1      5
#define DEFAULT_SYS_PLL_POST_2      2
#define DEFAULT_SYS_CLOCK_FREQ      DEFAULT_SYS_PLL_FREQ / DEFAULT_SYS_PLL_POST_1 / DEFAULT_SYS_PLL_POST_2  // 150MHz = 1500 / 5 / 2
#define DEFAULT_SYS_VOLTAGE         VREG_VOLTAGE_1_00


// protocol versions
#define PROTOCOL_VERSION_MAJ        1
#define PROTOCOL_VERSION_MIN        4


typedef struct {
  bool freeRun :    1; // enable free run of sensor
  bool calcStat :   1; // 
  bool bit2 :       1;
  bool bit3 :       1;
  bool bit4 :       1;
  bool bit5 :       1;
  bool bit6 :       1;
  bool bit7 :       1;
} ConfigBits;


union ConfigBitsUnion{
  ConfigBits bits;
  uint8_t all;
} configBitsUnion;


#endif //DEFINITIONS_H