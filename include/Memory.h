#ifndef __MEMORY_H

#include <cstdint>
#include <array>
#include "Buffer/StatisticBuffer.hpp"
#include "Definitions.h"


static const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static volatile bool awake = true;

/* Allocate memory for register */
volatile static uint8_t mainRegister[REGISTER_SIZE];

/* ### NON VOLATILE - PERMANENT VALUES ### */
static float* gainPv0       = (float *)(mainRegister + GAIN_PV0_OFFSET);
static float* gainPv1       = (float *)(mainRegister + GAIN_PV1_OFFSET);
static float* gainPv2       = (float *)(mainRegister + GAIN_PV2_OFFSET);
static float* gainPv3       = (float *)(mainRegister + GAIN_PV3_OFFSET);

static float* offsetPv0     = (float *)(mainRegister + OFFSET_PV0_OFFSET);
static float* offsetPv1     = (float *)(mainRegister + OFFSET_PV1_OFFSET);
static float* offsetPv2     = (float *)(mainRegister + OFFSET_PV2_OFFSET);
static float* offsetPv3     = (float *)(mainRegister + OFFSET_PV3_OFFSET);

static uint32_t *desiredCycleTimeUs     = (uint32_t *)(mainRegister + OFFSET_CYCLE_TIME);
static uint8_t *devAddress              = (uint8_t *)(mainRegister + OFFSET_ADDRESS);
static ConfigBitsUnion *config          = (ConfigBitsUnion *)(mainRegister + OFFSET_CONFIG_BITS); 
static uint32_t *clockKhz               = (uint32_t *)(mainRegister + OFFSET_CLOCK_KHZ);  // not used yet, TODO: Implement


/* ### VOLATILE - PROCESS VALUES ### */
static uint64_t* error      = (uint64_t *)(mainRegister + ERROR_OFFSET);
static uint64_t* status     = (uint64_t *)(mainRegister + STATUS_OFFSET);
static uint64_t* uid        = (uint64_t *)(mainRegister + UID_OFFSET);

static float* pv0           = (float *)(mainRegister + PV0_OFFSET);
static float* pv1           = (float *)(mainRegister + PV1_OFFSET);
static float* pv2           = (float *)(mainRegister + PV2_OFFSET);
static float* pv3           = (float *)(mainRegister + PV3_OFFSET);
std::array<float*, 4> processValues = {pv0, pv1, pv2, pv3};

static float* meanPv0       = (float *)(mainRegister + MEAN_PV0_OFFSET);
static float* meanPv1       = (float *)(mainRegister + MEAN_PV1_OFFSET);
static float* meanPv2       = (float *)(mainRegister + MEAN_PV2_OFFSET);
static float* meanPv3       = (float *)(mainRegister + MEAN_PV3_OFFSET);
std::array<float*, 4> meanValues = {meanPv0, meanPv1, meanPv2, meanPv3};

static float* stdDevPv0     = (float *)(mainRegister + STDEV_PV0_OFFSET);
static float* stdDevPv1     = (float *)(mainRegister + STDEV_PV1_OFFSET);
static float* stdDevPv2     = (float *)(mainRegister + STDEV_PV2_OFFSET);
static float* stdDevPv3     = (float *)(mainRegister + STDEV_PV3_OFFSET);
std::array<float*, 4> standardDeviations = {stdDevPv0, stdDevPv1, stdDevPv2, stdDevPv3};

static float* minPv0        = (float *)(mainRegister + MIN_PV0_OFFSET);
static float* minPv1        = (float *)(mainRegister + MIN_PV1_OFFSET);
static float* minPv2        = (float *)(mainRegister + MIN_PV2_OFFSET);
static float* minPv3        = (float *)(mainRegister + MIN_PV3_OFFSET);
std::array<float*, 4> minimumValues = {minPv0, minPv1, minPv2, minPv3};

static float* maxPv0        = (float *)(mainRegister + MAX_PV0_OFFSET);
static float* maxPv1        = (float *)(mainRegister + MAX_PV1_OFFSET);
static float* maxPv2        = (float *)(mainRegister + MAX_PV2_OFFSET);
static float* maxPv3        = (float *)(mainRegister + MAX_PV3_OFFSET);
std::array<float*, 4> maximumValues = {maxPv0, maxPv1, maxPv2, maxPv3};

// define ringbuffer (circular buffer) for each process value
Xerxes::StatisticBuffer<float> rbpv0(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv1(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv2(RING_BUFFER_LEN);
Xerxes::StatisticBuffer<float> rbpv3(RING_BUFFER_LEN);

std::array<Xerxes::StatisticBuffer<float>,4> ringBuffers = {rbpv0, rbpv1, rbpv2, rbpv3};


#endif // !__MEMORY_H