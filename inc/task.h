#ifndef __TASK_H
#define __TASK_H
#include "lora.h"

typedef enum
{
    TaskEmpty,
    TICK,                  //20ms的TICK
    LORA_RECV_DATA,        //LoRa串口收到数据
    DEV_RECV_DATA,         //Device串口收到数据
    LORA_SEND_COMPLETE,    //LoRa口发送数据完成
    DEV_SEND_COMPLETE,     //Device口发送数据完成
    LORA_DATA_SEND,        //需要从LoRa发送数据出去
    START_DELAY_TASK,      //启动延迟任务
    ATCMD_RESTARTEND,      //LoRa进入AT模式已经重启完成
    ENTER_ATMODLE_TIMEOUT, //LoRa进入AT命令超时
    EXIT_AT_TIMEOUT,       //LoRa退出At命令模式超时
    ENTER_GPRS_AT,         //进入GPRS的AT命令模式
    ENTER_GPRS_AT_TIMEOUT, //进入GPRS的AT命令模式超时
    CONF_DevType_GW,       //设置设备类型为GPRS网关
    CONF_LORA_PARM,        //初始化设备类型后，设置Lora的参数设
    GW_Register_Task,      //网关注册任务
    LoraRegister_Task,     //Lora注册，获取自己的配置
    GetLoraDevParam_Task,  //Lora连接的设备注册，获取要连接设备的串口配置参数
    HeartBeat_Task,        //心跳任务
    SyncTime_Task,         //时间同步任务

} TaskType;

extern void HandleTask();
extern void PushTask(TaskType t);
extern void InitTask();
extern TaskType TaskList[];

#endif // !__TASK__H