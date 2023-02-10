#ifndef __MEMORY_H
#define __MEMORY_H

#include <cstdint>
#include <array>
#include "Buffer/StatisticBuffer.hpp"
#include "Definitions.h"


const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET); ///< Flash memory contents
volatile bool awake = true;

/* Allocate memory for register */
volatile uint8_t mainRegister[REGISTER_SIZE];  ///< Main register allocation for the device

/* ### NON VOLATILE - PERMANENT VALUES ### */
float* gainPv0       = (float *)(mainRegister + GAIN_PV0_OFFSET);
float* gainPv1       = (float *)(mainRegister + GAIN_PV1_OFFSET);
float* gainPv2       = (float *)(mainRegister + GAIN_PV2_OFFSET);
float* gainPv3       = (float *)(mainRegister + GAIN_PV3_OFFSET);

float* offsetPv0     = (float *)(mainRegister + OFFSET_PV0_OFFSET);
float* offsetPv1     = (float *)(mainRegister + OFFSET_PV1_OFFSET);
float* offsetPv2     = (float *)(mainRegister + OFFSET_PV2_OFFSET);
float* offsetPv3     = (float *)(mainRegister + OFFSET_PV3_OFFSET);

uint32_t *desiredCycleTimeUs     = (uint32_t *)(mainRegister + OFFSET_DESIRED_CYCLE_TIME);  ///< Desired cycle time of sensor loop in microseconds
uint8_t *devAddress              = (uint8_t *)(mainRegister + OFFSET_ADDRESS);  ///< Address of the device (1 byte)
ConfigBitsUnion *config          = (ConfigBitsUnion *)(mainRegister + OFFSET_CONFIG_BITS);  ///< Config bits of the device (1 byte)
uint32_t *netCycleTimeUs         = (uint32_t *)(mainRegister + OFFSET_NET_CYCLE_TIME);  ///< Actual cycle time of measurement loop in microseconds


/* ### VOLATILE - PROCESS VALUES ### */

float* pv0           = (float *)(mainRegister + PV0_OFFSET);    ///< Pointer to process value 0
float* pv1           = (float *)(mainRegister + PV1_OFFSET);    ///< Pointer to process value 1
float* pv2           = (float *)(mainRegister + PV2_OFFSET);    ///< Pointer to process value 2
float* pv3           = (float *)(mainRegister + PV3_OFFSET);    ///< Pointer to process value 3
std::array<float*, 4> processValues = {pv0, pv1, pv2, pv3}; ///< Array of pointers to process values

float* meanPv0       = (float *)(mainRegister + MEAN_PV0_OFFSET);
float* meanPv1       = (float *)(mainRegister + MEAN_PV1_OFFSET);
float* meanPv2       = (float *)(mainRegister + MEAN_PV2_OFFSET);
float* meanPv3       = (float *)(mainRegister + MEAN_PV3_OFFSET);
std::array<float*, 4> meanValues = {meanPv0, meanPv1, meanPv2, meanPv3};    ///< Array of pointers to mean values

float* stdDevPv0     = (float *)(mainRegister + STDDEV_PV0_OFFSET);
float* stdDevPv1     = (float *)(mainRegister + STDDEV_PV1_OFFSET);
float* stdDevPv2     = (float *)(mainRegister + STDDEV_PV2_OFFSET);
float* stdDevPv3     = (float *)(mainRegister + STDDEV_PV3_OFFSET);
std::array<float*, 4> standardDeviations = {stdDevPv0, stdDevPv1, stdDevPv2, stdDevPv3};    ///< Array of pointers to standard deviation values

float* minPv0        = (float *)(mainRegister + MIN_PV0_OFFSET);
float* minPv1        = (float *)(mainRegister + MIN_PV1_OFFSET);
float* minPv2        = (float *)(mainRegister + MIN_PV2_OFFSET);
float* minPv3        = (float *)(mainRegister + MIN_PV3_OFFSET);
std::array<float*, 4> minimumValues = {minPv0, minPv1, minPv2, minPv3};   ///< Array of pointers to minimum values

float* maxPv0        = (float *)(mainRegister + MAX_PV0_OFFSET);
float* maxPv1        = (float *)(mainRegister + MAX_PV1_OFFSET);
float* maxPv2        = (float *)(mainRegister + MAX_PV2_OFFSET);
float* maxPv3        = (float *)(mainRegister + MAX_PV3_OFFSET);
std::array<float*, 4> maximumValues = {maxPv0, maxPv1, maxPv2, maxPv3};  ///< Array of pointers to maximum values

/* ### READ ONLY VALUES ### */

/** @brief 0x55AA55AA = unlocked, anything else = locked */
uint32_t* memUnlocked   = (uint32_t *)(mainRegister + MEM_UNLOCKED_OFFSET);

uint64_t* error      = (uint64_t *)(mainRegister + ERROR_OFFSET);   ///< Error register, holds error codes
uint64_t* status     = (uint64_t *)(mainRegister + STATUS_OFFSET);  ///< Status register, holds status codes
uint64_t* uid        = (uint64_t *)(mainRegister + UID_OFFSET);     ///< Unique ID of the device


/* ### MESSAGE STRING MEMORY ### */

char* message    = (char *)(mainRegister + MESSAGE_OFFSET); ///< Message string, holds messages (debug, info, warning, error)


// define ringbuffer (circular buffer) for each process value
Xerxes::StatisticBuffer<float> rbpv0(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv1(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv2(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv3(RING_BUFFER_LEN);

std::array<Xerxes::StatisticBuffer<float>,4> ringBuffers = {rbpv0, rbpv1, rbpv2, rbpv3};    ///< Array of ring buffers


#endif // !__MEMORY_H