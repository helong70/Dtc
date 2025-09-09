#ifndef DTC_CFG_H
#define DTC_CFG_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DTC_STATE_NO_FAULT = 0,
    DTC_STATE_PENDING,
    DTC_STATE_CONFIRMED
} DtcState_t;


typedef enum {
    DTC_FAULT_OVER_VOLTAGE,
    DTC_FAULT_UNDER_VOLTAGE,
    DTC_FAULT_SHORT_CIRCUIT,
    DTC_FAULT_OPEN_CIRCUIT,
    /* 其他故障类型可以在这里添加 */
    DTC_NUM,

} DtcFaultType_t;

typedef enum {
    DTC_CONFIG_OUT_OF_RANGE,
    /* 其他配置类型可以在这里添加 */
} DtcConfigType_t;
typedef enum {
    DTC_UP_OF_RANGE,
    DTC_DOWN_OF_RANGE,
    DTC_BETWEEN_RANGE,
    /* 其他故障类型可以在这里添加 */
} DtcOutofRangeType_t;

typedef struct {
    uint32_t * pvalue;              /* 监测值指针 */
    uint32_t highThreshold;      /* 过压阈值 */
    uint32_t highHyst;           /* 过压迟滞 */
    uint32_t lowThreshold;       /* 欠压阈值 */
    uint32_t lowHyst;            /* 欠压迟滞 */
    DtcOutofRangeType_t rangeType; /* 范围类型 */
} DtcOutofRangeConfig_t;



typedef union {
    DtcOutofRangeConfig_t outOfRangeConfig;
    /* 其他故障类型的配置结构体可以在这里添加 */
} DtcConfig_t;



typedef struct {
    DtcFaultType_t faultType;
    DtcConfigType_t configType;
    DtcConfig_t config;
    uint16_t detectCountToConfirm; /* 连续次数达到才确认 */
    uint16_t clearCountToClear;    /* 连续正常次数达到才清除 */
    uint16_t detectCounter;  /* 用于确认故障 */
    uint16_t clearCounter;   /* 用于清除故障 */
    DtcState_t state;
} DtcChannel_t;

extern DtcChannel_t DtcChannelCfg[DTC_NUM];

void Dtc_OutOfRange(DtcChannel_t* chan);
#endif // DTC_CFG_H