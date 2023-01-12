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
// memory offset of the gain of the process values
#define GAIN_PV0_OFFSET             0
#define GAIN_PV1_OFFSET             4
#define GAIN_PV2_OFFSET             8
#define GAIN_PV3_OFFSET             12

// memory offset of the offset of the process values
#define OFFSET_PV0_OFFSET           16
#define OFFSET_PV1_OFFSET           20
#define OFFSET_PV2_OFFSET           24
#define OFFSET_PV3_OFFSET           28

// memory offset of cycle time in microseconds
#define OFFSET_DESIRED_CYCLE_TIME   32
#define OFFSET_NET_CYCLE_TIME       36

// memory offset of address of the device
#define OFFSET_ADDRESS              44

// memory offset of config bits
#define OFFSET_CONFIG_BITS          40


//volatile range
// memory offset of the status of the device
#define STATUS_OFFSET               NON_VOLATILE_SIZE + 0       // 256
// memory offset of the errors of the device
#define ERROR_OFFSET                NON_VOLATILE_SIZE + 8       // 264
// memory offset of the uid of the device
#define UID_OFFSET                  NON_VOLATILE_SIZE + 16      // 272

// memory offset of the process values
#define PV0_OFFSET                  NON_VOLATILE_SIZE + 20      // 276
#define PV1_OFFSET                  NON_VOLATILE_SIZE + 24      // 280
#define PV2_OFFSET                  NON_VOLATILE_SIZE + 28      // 284
#define PV3_OFFSET                  NON_VOLATILE_SIZE + 32      // 288

// memory offset of the mean values of the process values
#define MEAN_PV0_OFFSET             NON_VOLATILE_SIZE + 36      // 292
#define MEAN_PV1_OFFSET             NON_VOLATILE_SIZE + 40      // 296
#define MEAN_PV2_OFFSET             NON_VOLATILE_SIZE + 44      // 300 
#define MEAN_PV3_OFFSET             NON_VOLATILE_SIZE + 48      // 304
 
// memory offset of the standard deviation of the process values
#define STDEV_PV0_OFFSET            NON_VOLATILE_SIZE + 52      // 308
#define STDEV_PV1_OFFSET            NON_VOLATILE_SIZE + 56      // 312
#define STDEV_PV2_OFFSET            NON_VOLATILE_SIZE + 60      // 316
#define STDEV_PV3_OFFSET            NON_VOLATILE_SIZE + 64      // 320

// memory offset of the minimum and maximum values of the process values
#define MIN_PV0_OFFSET              NON_VOLATILE_SIZE + 68      // 324
#define MIN_PV1_OFFSET              NON_VOLATILE_SIZE + 72      // 328
#define MIN_PV2_OFFSET              NON_VOLATILE_SIZE + 76      // 332
#define MIN_PV3_OFFSET              NON_VOLATILE_SIZE + 80      // 336
 
#define MAX_PV0_OFFSET              NON_VOLATILE_SIZE + 84      // 340
#define MAX_PV1_OFFSET              NON_VOLATILE_SIZE + 88      // 344
#define MAX_PV2_OFFSET              NON_VOLATILE_SIZE + 92      // 348
#define MAX_PV3_OFFSET              NON_VOLATILE_SIZE + 96      // 352

/* config masks */
/* If true use free run, if false: wait for sync packet */
#define MASK_CONFIG_FREE_RUN        1<<0
/* if true, enable automatic calculation of the statistics */
#define MASK_CONFIG_CALC_STATS      1<<1


/* Default values */
#define DEFAULT_CYCLE_TIME_US       10'000      // 10 ms
#define DEFAULT_WATCHDOG_DELAY      100         // ms

/* Default clocks */
#define DEFAULT_USB_CLOCK_FREQ      48 * MHZ
#define DEFAULT_XOSC_CLOCK_FREQ     12 * MHZ

#define DEFAULT_ADC_CLOCK_SRC       CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB // maybe use XOSC
#define DEFAULT_ADC_CLOCK_FREQ      DEFAULT_USB_CLOCK_FREQ
#define DEFAULT_PERI_CLOCK_SRC      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB
#define DEFAULT_PERI_CLOCK_FREQ     DEFAULT_USB_CLOCK_FREQ

/* system clock */
#define DEFAULT_SYS_CLOCK_SRC       CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS
#define DEFAULT_SYS_CLOCK_SRC_LP    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF
#define DEFAULT_SYS_PLL_FREQ        756 * MHZ
#define DEFAULT_SYS_PLL_POST_1      6
#define DEFAULT_SYS_PLL_POST_2      1
#define DEFAULT_SYS_CLOCK_FREQ      DEFAULT_SYS_PLL_FREQ / DEFAULT_SYS_PLL_POST_1 / DEFAULT_SYS_PLL_POST_2  // 756MHz / 6 / 1 = 126MHz
#define DEFAULT_SYS_CLOCK_FREQ_LP   DEFAULT_XOSC_CLOCK_FREQ
#define DEFAULT_SYS_VOLTAGE         VREG_VOLTAGE_DEFAULT // 1.1V
#define DEFAULT_SYS_VOLTAGE_LP      VREG_VOLTAGE_0_90    // 0.90V


// protocol versions
#define PROTOCOL_VERSION_MAJ        1
#define PROTOCOL_VERSION_MIN        4


typedef struct {
  bool freeRun :    1; // enable free run of sensor
  bool calcStat :   1; // enable calculation of statistics
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