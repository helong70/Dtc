#include "Dtc_cfg.h"


DtcChannel_t DtcChannelCfg[DTC_NUM] = {
{
    .faultType = DTC_FAULT_OVER_VOLTAGE,
    .config = {
        .outOfRangeConfig = {
            .highThreshold = 32000,
            .highHyst = 31000,
            .lowThreshold = 2000,
            .lowHyst = 2100
        }
    },
    .detectCountToConfirm = 3,
    .clearCountToClear = 5,
    .detectCounter = 0,
    .clearCounter = 0,
    .state = DTC_STATE_NO_FAULT,
}
};

void Dtc_OutOfRange(DtcChannel_t* chan)
{
    uint32_t value = *(chan->config.outOfRangeConfig.pvalue); // 这里应获取实际的监测值
    bool faultDetected = false;

    switch (chan->config.outOfRangeConfig.rangeType) {
        case DTC_UP_OF_RANGE:
            if (value > chan->config.outOfRangeConfig.highThreshold) {
                faultDetected = true;
            } else if (value < chan->config.outOfRangeConfig.highHyst) {
                faultDetected = false;
            }
            break;
        case DTC_DOWN_OF_RANGE:
            if (value < chan->config.outOfRangeConfig.lowThreshold) {
                faultDetected = true;
            } else if (value > chan->config.outOfRangeConfig.lowHyst) {
                faultDetected = false;
            }
            break;
        case DTC_BETWEEN_RANGE:
            if (value < chan->config.outOfRangeConfig.lowThreshold || value > chan->config.outOfRangeConfig.highThreshold) {
                faultDetected = true;
            } else if (value > (chan->config.outOfRangeConfig.lowHyst) &&
                       value < (chan->config.outOfRangeConfig.highHyst)) {
                faultDetected = false;
            }
            break;
        default:
            break;
    }

    if (faultDetected && (chan->state == DTC_STATE_NO_FAULT||chan->state == DTC_STATE_PENDING)) {
        chan->detectCounter++;
        chan->clearCounter = 0;
        if (chan->detectCounter >= chan->detectCountToConfirm) {
            chan->state = DTC_STATE_CONFIRMED;
        } else {
            chan->state = DTC_STATE_PENDING;
        }
    } else if (!faultDetected && chan->state == DTC_STATE_CONFIRMED) {
        chan->clearCounter++;
        chan->detectCounter = 0;
        if (chan->clearCounter >= chan->clearCountToClear) {
            chan->state = DTC_STATE_NO_FAULT;
        }
    }
    else {
        chan->detectCounter = 0;
        chan->clearCounter = 0;
        if (chan->state == DTC_STATE_PENDING) {
            chan->state = DTC_STATE_NO_FAULT;
        }
    }
}               