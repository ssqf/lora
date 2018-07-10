// 任务列表

#include "lora.h"

TaskNode *taskList;
TaskNode *taskTail;

void InitTask()
{
    taskList = NULL;
    taskTail = NULL;
}

void pushTask(Task *t)
{
    TaskNode *node = (TaskNode *)malloc(sizeof(TaskNode));
    node->task = t;
    node->next = NULL;
    taskTail->next = node;
    taskTail = taskTail->next;
    if (taskList == NULL)
    {
        taskList = node;
    }
}

static TaskNode *popTask()
{
    if (taskList == NULL)
    {
        return NULL;
    }

    TaskNode *node = taskList;
    taskList = taskList->next;
    return node;
}

static void delNode(TaskNode *node)
{
    free(node->task->data);
    node->task->data = NULL;
    free(node);
    node = NULL;
}

void handleTask()
{
    TaskNode *node = NULL;
    Task *task = NULL;
    while (1)
    {
        node = popTask();
        if (node == NULL)
        {
            continue;
        }

        task = node->task;
        switch (task->type)
        {
        case LORA_RECV_DATA:
            //SendDevice(task->data, task->dataLen);
            //SendLora(task->data, task->dataLen);
            break;
        case DEV_RECV_DATA:
            //SendLora(task->data, task->dataLen);
            SendDevice(task->data, task->dataLen);
            break;
        }
        delNode(node);
    }
}