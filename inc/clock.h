#ifndef __CLOCK_H
#define __CLOCK_H

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
#endif // !__CLOCK_H
