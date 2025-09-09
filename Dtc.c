#include "Dtc.h"
#include "Dtc_cfg.h" 
DtcFault_t DtcStatus[DTC_NUM] = {0};

void Dtc_Init(void)
{


}

void Dtc_Clear( DtcFaultType_t dtcId)
{
    DtcChannel_t* chan = &DtcChannelCfg[dtcId];
    chan->detectCounter = 0;
    chan->clearCounter = 0;
    chan->state = DTC_STATE_NO_FAULT;
}
            
void Dtc_Report( DtcFaultType_t dtcId)
{
    DtcChannel_t* chan = &DtcChannelCfg[dtcId];
    DtcStatus[dtcId]= chan->state == DTC_STATE_CONFIRMED ? DTC_FAULT_DETECTED : DTC_NO_FAULT;
}

void Dtc_Main(void)
{
    DtcChannel_t* chan= DtcChannelCfg;
    for (int i = 0; i < DTC_NUM; i++) {
        switch(chan[i].configType) {
            case DTC_CONFIG_OUT_OF_RANGE:
                Dtc_OutOfRange(&chan[i]);
                break;
                /* 其他配置类型的处理函数可以在这里添加 */
            default:
                break;
        }
        Dtc_Report(i);
    }
}