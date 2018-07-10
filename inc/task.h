#ifndef __TASK_H
#define __TASK_H
#include "lora.h"

typedef enum
{
    TICK,      //接收数据超时
    RECV_FULL, //接收数据达到最大
    LORA_RECV_DATA,
    DEV_RECV_DATA,
} TaskType;

typedef struct
{
    TaskType type;
    uint8_t *data;
    uint8_t dataLen;
} Task;

typedef struct Node
{
    Task *task;
    struct Node *next;
} TaskNode;
extern void InitTask();
extern void handleTask();

#endif // !__TASK__H