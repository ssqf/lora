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
    //TIM1_PrescalerConfig(1, TIM1_PSCReloadMode_Update);
    TIM1_TimeBaseInit(15, TIM1_CounterMode_Down, TIM1_PERIOD, 0); //15+1
    //TIM1_SetAutoreload(TIM1_PERIOD);
    TIM1_ARRPreloadConfig(ENABLE);
    TIM1_ITConfig(TIM1_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM1_UPD_OVF_TRG_IRQn, ITC_PriorityLevel_0);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
    TIM1_Cmd(ENABLE);
}

void SyncTime(uint32_t s, uint8_t t, uint16_t counter)
{
    Second = s;
    TickNum = t;
    TIM1_SetCounter(counter);
}

void InitDelayTimer()
{
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Down, 1000); //1M 1000 1ms
    //TIM2_SetAutoreload(1000);
    TIM2_ARRPreloadConfig(ENABLE);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQn, ITC_PriorityLevel_3);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    //TIM1_Cmd(ENABLE);
}

#define delayTaskSize 10
#define invalidCurrIndex -1 //-1无任务
DelayTask delayTaskList[delayTaskSize];
int curTaskIndex = invalidCurrIndex;

bool DelaySendTask(uint16_t ms, TaskType t)
{
    for (uint8_t i = 0; i < delayTaskSize; i++)
    {
        if (!delayTaskList[i].used)
        {
            delayTaskList[i].t = t;
            delayTaskList[i].used = TRUE;

            if (!(TIM2->CR1 & TIM_CR1_CEN))
            {
                delayTaskList[i].delay = ms;
                PushTask(START_DELAY_TASK);
            }
            else
            {
                delayTaskList[i].delay = ms - TIM2_GetCounter();
            }

            return TRUE;
        }
    }
    return FALSE;
}

DelayTask *GetMinTask()
{
    DelayTask *minTask = NULL;
    //curTaskIndex = invalidCurrIndex;
    for (uint8_t i = 0; i < delayTaskSize; i++)
    {
        if (delayTaskList[i].used)
        {
            if (NULL == minTask)
            {
                minTask = &delayTaskList[i];
                curTaskIndex = i;
            }
            else if (minTask->delay > delayTaskList[i].delay)
            {
                minTask = &delayTaskList[i];
                curTaskIndex = i;
            }
        }
    }

    if (NULL == minTask)
    {
        curTaskIndex = invalidCurrIndex;
    }

    return minTask;
}

DelayTask *GetCurrTask()
{
    if (invalidCurrIndex == curTaskIndex)
    {
        return NULL;
    }
    return &delayTaskList[curTaskIndex];
}

//TODO:延迟任务队列实现

void StartDelayTask() //5ms终端发送lora数据
{
    DelayTask *task = GetMinTask();
    if (!task->used)
    {
        return;
    }
    StartNextDelyaTask(task->delay);
}

void StartNextDelyaTask(uint16_t time)
{
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Down, time); //1M 1000 1ms
    //TIM2_SetAutoreload(1000);
    TIM2_ARRPreloadConfig(DISABLE);
    ITC_SetSoftwarePriority(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQn, ITC_PriorityLevel_3);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    TIM2_ClearITPendingBit(TIM2_IT_Update);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    TIM2_Cmd(ENABLE);
}