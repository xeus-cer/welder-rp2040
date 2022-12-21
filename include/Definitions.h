#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "hardware/flash.h"

#define DEFAULT_BAUDRATE    250000

#define REGISTER_SIZE       512
#define NON_VOLATILE_SIZE   FLASH_PAGE_SIZE // 256
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


//volatile range
#define STATUS_OFFSET       NON_VOLATILE_SIZE + 0       // 256
#define ERROR_OFFSET        NON_VOLATILE_SIZE + 8       // 264
#define UID_OFFSET          NON_VOLATILE_SIZE + 16      // 272

#define PV0_OFFSET          NON_VOLATILE_SIZE + 24      // 280
#define PV1_OFFSET          NON_VOLATILE_SIZE + 32      // 288
#define PV2_OFFSET          NON_VOLATILE_SIZE + 40      // 296
#define PV3_OFFSET          NON_VOLATILE_SIZE + 48      // 304

#define MEAN_PV0_OFFSET     NON_VOLATILE_SIZE + 56      // 312
#define MEAN_PV1_OFFSET     NON_VOLATILE_SIZE + 64      // 320
#define MEAN_PV2_OFFSET     NON_VOLATILE_SIZE + 72      // 328 
#define MEAN_PV3_OFFSET     NON_VOLATILE_SIZE + 80      // 336
 
#define STDEV_PV0_OFFSET    NON_VOLATILE_SIZE + 88      // 344
#define STDEV_PV1_OFFSET    NON_VOLATILE_SIZE + 96      // 352
#define STDEV_PV2_OFFSET    NON_VOLATILE_SIZE + 104     // 360
#define STDEV_PV3_OFFSET    NON_VOLATILE_SIZE + 112     // 368

#define MIN_PV0_OFFSET      NON_VOLATILE_SIZE + 120     // 376
#define MIN_PV1_OFFSET      NON_VOLATILE_SIZE + 128     // 384
#define MIN_PV2_OFFSET      NON_VOLATILE_SIZE + 136     // 392
#define MIN_PV3_OFFSET      NON_VOLATILE_SIZE + 144     // 400
 
#define MAX_PV0_OFFSET      NON_VOLATILE_SIZE + 152     // 408
#define MAX_PV1_OFFSET      NON_VOLATILE_SIZE + 160     // 416
#define MAX_PV2_OFFSET      NON_VOLATILE_SIZE + 168     // 424
#define MAX_PV3_OFFSET      NON_VOLATILE_SIZE + 176     // 432

/* config masks */
/* If true use free run, if false: wait for sync packet */
#define MASK_CONFIG_FREE_RUN        1<<0
/* If true, enable low power sleep and disable USB */
#define MASK_CONFIG_LOW_POWER       1<<1


/* Default values */
#define DEFAULT_CYCLE_TIME_US       10000  // 100ms

// protocol versions
#define PROTOCOL_VERSION_MAJ        1
#define PROTOCOL_VERSION_MIN        4


typedef struct {
  bool freeRun :    1 = false;
  bool lowPower :   1 = false;
  bool bit2 :       1 = false;
  bool bit3 :       1 = false;
  bool bit4 :       1 = false;
  bool bit5 :       1 = false;
  bool bit6 :       1 = false;
  bool firstRun :   1 = true;
} ConfigBits;


#endif //DEFINITIONS_H