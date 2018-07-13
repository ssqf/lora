// 任务列表

#include "lora.h"

#define LISTSIZE 20
int8_t head = 0;
int8_t tail = 0;
int8_t listLen = 0;

TaskType TaskList[LISTSIZE];

void InitTask()
{
    head = 0;
    tail = 0;
    listLen = 0;
}

void PushTask(TaskType t)
{
    if (listLen >= LISTSIZE)
    {
        //Reset;
        return;
    }

    TaskList[tail] = t;
    tail++;
    listLen++;

    if (tail >= LISTSIZE && listLen <= LISTSIZE)
    {
        tail = 0;
    }
}

TaskType popTask()
{
    if (0 >= listLen)
    {
        return TaskEmpty;
    }
    TaskType t = TaskList[head];
    head++;
    if (head >= LISTSIZE)
    {
        head = 0;
    }
    listLen--;
    return t;
}
bool IsEmptyTaskList()
{
    if (0 >= listLen)
    {
        return TRUE;
    }
    return FALSE;
}

void HandleTask()
{
    TaskType task;
    char str[50] = "second=0000,tick=00\n";
    while (1)
    {
        while (IsEmptyTaskList()) //队列不为空
            ;
        task = popTask();
        switch (task)
        {
        case TICK:
            if (TickNum == 1)
            {
                str[7] = '0' + Second / 1000 % 10;
                str[8] = '0' + Second / 100 % 10;
                str[9] = '0' + Second / 10 % 10;
                str[10] = '0' + Second % 10;
                str[17] = '0' + TickNum / 10;
                str[18] = '0' + TickNum % 10;
                SendDevice((uint8_t *)str, (uint8_t)strlen(str));
            }
            break;

        case LORA_RECV_DATA:
            ResetDMARx(LoraUSART);
            SendDevice("LORA_RECV_DATA\n", 15);
            //SendDevice(task->data, task->dataLen);
            //SendLora(task->data, task->dataLen);

            break;
        case DEV_RECV_DATA:
            ResetDMARx(DevUSART);
            SendDevice("DEV_RECV_DATA\n", 14);
            //SendLora(task->data, task->dataLen);
            //SendDevice(task->data, task->dataLen);
            break;
        case DEV_SEND_COMPLETE:
            SetRS485CTL(RESET);
            //SendDevice("DEV_SEND_COMPLETE\n", 19);
            break;
        case LORA_SEND_COMPLETE:
            //SendDevice("LORA_SEND_COMPLETE\n", 20);
            break;
        }
    }
}