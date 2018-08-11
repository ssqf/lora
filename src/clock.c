// 时钟，10小时清零，16位定时器T2，20ms中断一次处理事情
// 每秒分三个阶段；1,20ms-300ms定时类设备发送给网关数据;2,320ms-600ms网关给设备发送数据；3，620ms-1000ms告警类设备
//
#include "lora.h"
uint32_t Second;
uint8_t TickNum;
static void time2Config(void);
//static void dalayTimer1Init();
void restartTask(uint16_t time);

void InitClock()
{
    time2Config();
    //dalayTaskTimer1Init();
}

static void time2Config(void)
{
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Down, TIM2_PERIOD);
    TIM2_ARRPreloadConfig(ENABLE);
    ITC_SetSoftwarePriority(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQn, ITC_PriorityLevel_2);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    TIM2_ClearITPendingBit(TIM2_IT_Update);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    TIM2_Cmd(ENABLE);
}

void SyncTimeToLoacl(uint32_t s, uint8_t t, uint16_t counter)
{
    Second = s;
    TickNum = t;
    TIM2_SetCounter(counter);
}

// static void dalayTaskTimer1Init()
// {
//     TIM1_TimeBaseInit(16000 - 1, TIM1_CounterMode_Down, 0, 0); //15+1 分频 time*1ms
//     //TIM1_UpdateRequestConfig(TIM1_UpdateSource_Regular);
//     TIM1_ARRPreloadConfig(DISABLE);
//     //ITC_SetSoftwarePriority(TIM1_UPD_OVF_TRG_IRQn, ITC_PriorityLevel_3);
//     CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE); //在使能前清除标准，不然立即进中断
//     TIM1_ClearITPendingBit(TIM1_IT_Update);
//     //TIM1_ClearFlag(TIM1_FLAG_Update);
//     TIM1_ITConfig(TIM1_IT_Update, ENABLE);
//     TIM1_Cmd(DISABLE);
// }

#define delayTaskSize 20
#define invalidCurrIndex -1 //-1无任务
DelayTask delayTaskList[delayTaskSize];
int curTaskIndex = invalidCurrIndex;

bool DelaySendTask(uint16_t ms, TaskType t)
{
    Debug("DelaySendTask:%ums,%u", ms, t);
    for (uint8_t i = 0; i < delayTaskSize; i++)
    {
        if (!delayTaskList[i].used) //找到一个未被使用的延迟任务位置
        {
            delayTaskList[i].t = t; //将任务保存
            delayTaskList[i].used = TRUE;

            if (!(TIM1->CR1 & TIM_CR1_CEN)) //当前延迟任务队列没有运行
            {
                delayTaskList[i].delay = ms;
                PushTask(START_DELAY_TASK);
            }
            else
            {
                uint16_t counter = TIM1_GetCounter();
                Debug("ms:%u,counter:%u", ms, counter);
                if (ms <= counter) //延时小于当前延时，则停止当前延时，重启新的最小延时
                {
                    DelayTask *t = GetCurrTask();
                    t->delay -= counter;
                    delayTaskList[i].delay = ms; //0 timer终端不能启动
                    t = GetMinTask();
                    restartTask(t->delay);
                }
                else
                {
                    delayTaskList[i].delay = ms - counter;
                }
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
    TIM1_TimeBaseInit(160 - 1, TIM1_CounterMode_Down, time, 100); //15+1 分频 time*1ms
    //TIM1_PrescalerConfig(16000 - 1, TIM1_PSCReloadMode_Immediate); //分频不能太大，不是说的0-65535
    //TIM1_UpdateRequestConfig(TIM1_UpdateSource_Regular);
    //TIM1_UpdateDisableConfig(ENABLE);
    TIM1_ARRPreloadConfig(DISABLE);
    ITC_SetSoftwarePriority(TIM1_UPD_OVF_TRG_IRQn, ITC_PriorityLevel_3);
    //CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
    TIM1_ClearITPendingBit(TIM1_IT_Update); //在使能前清除标准，不然立即进中断
    TIM1_ClearFlag(TIM1_FLAG_Update);
    TIM1_ITConfig(TIM1_IT_Update, ENABLE);
    TIM1_Cmd(ENABLE);
}

void restartTask(uint16_t time)
{
    TIM1_Cmd(DISABLE);
    StartNextDelyaTask(time);
}