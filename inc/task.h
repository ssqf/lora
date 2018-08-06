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
    START_DELAY_TASK,
    ATCMD_RESTARTEND,
    ENTER_ATMODLE_TIMEOUT,
    EXIT_AT_TIMEOUT,
    ENTER_GPRS_AT,
    ENTER_GPRS_AT_TIMEOUT

} TaskType;

extern void HandleTask();
extern void PushTask(TaskType t);
extern void InitTask();
extern TaskType TaskList[];

#endif // !__TASK__H