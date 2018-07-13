#ifndef __TASK_H
#define __TASK_H
#include "lora.h"

typedef enum
{
    TaskEmpty,
    TICK, //接收数据超时
    LORA_RECV_DATA,
    DEV_RECV_DATA,
    LORA_SEND_COMPLETE,
    DEV_SEND_COMPLETE,
    LORA_DATA_SEND,

} TaskType;

extern void HandleTask();
extern void PushTask(TaskType t);
extern void InitTask();
extern TaskType TaskList[];

#endif // !__TASK__H