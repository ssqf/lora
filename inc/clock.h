#ifndef __CLOCK_H
#define __CLOCK_H

//#define TIM2_PERIOD 20000 //20ms 1MHz 16分频,1us单位
#define TIM2_PERIOD 50000 //50ms 1MHz 16分频,1us单位
#define TickPerSecond (1000000 / TIM2_PERIOD)
#define COUNTER5S (5 * TickPerSecond)
#define COUNTER1S (1 * TickPerSecond)
typedef struct
{
    uint16_t delay;
    TaskType t;
    bool used;
} DelayTask;

extern void InitClock();
extern void Delay5ms(); //5ms终端发送lora数据
extern uint32_t Second;
extern uint8_t TickNum;
extern bool DelaySendTask(uint16_t ms, TaskType t);
extern DelayTask *GetMinTask();
extern DelayTask *GetCurrTask();
extern void StartDelayTask();
extern void StartNextDelyaTask(uint16_t time);
extern void SyncTimeToLoacl(uint32_t s, uint8_t t, uint16_t counter);
#endif // !__CLOCK_H
