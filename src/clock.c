// 时钟，10小时清零，16位定时器T2，20ms中断一次处理事情
// 每秒分三个阶段；1,20ms-300ms定时类设备发送给网关数据;2,320ms-600ms网关给设备发送数据；3，620ms-1000ms告警类设备
//
#include "lora.h"
#define TIM1_PERIOD 20000 //20ms 1MHz 16分频
uint32_t Second = 0;
uint8_t TickNum = 0;
static void time1Config(void);

void InitClock()
{
    time1Config();
}

static void time1Config(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
    TIM1_TimeBaseInit(15, TIM1_CounterMode_Down, TIM1_PERIOD, 0); //15+1
    TIM1_SetAutoreload(TIM1_PERIOD);
    TIM1_ARRPreloadConfig(ENABLE);
    TIM1_ITConfig(TIM1_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM1_UPD_OVF_TRG_IRQn, ITC_PriorityLevel_0);
    TIM1_Cmd(ENABLE);
}

void SyncTime(uint32_t s, uint8_t t, uint16_t counter)
{
    Second = s;
    TickNum = t;
    TIM1_SetCounter(counter);
}