#include "lora.h"

void SetLoraSerial()
{
}

void GetLoraSerial()
{
}

void SetLoraAddr(uint16_t addr)
{
}

void SetLoraChan(uint8_t chan)
{
}

void initLora()
{
    GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3, GPIO_Mode_Out_PP_High_Fast); //Host_WAKE，M_WAKE，M_Reload,M_RST,
    GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Fast);                                        //CTRL_REDAY

    GPIO_WriteBit(GPIOB, GPIO_Pin_0, RESET); //Host_WAKE 默认低电平，拉高5ms发送数据
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, SET);   //M_WAKE 默认高电平，下降沿唤醒
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, SET);   //M_Reload 默认高电平，拉低3s以上恢复出厂设置
    GPIO_WriteBit(GPIOB, GPIO_Pin_3, SET);   //M_RST 默认高电平，拉低5ms复位
    GPIO_WriteBit(GPIOD, GPIO_Pin_0, SET);   //M_Reload 默认高电平，拉低3s以上恢复出厂设置
}

void SetHostWakeState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, state);
}
void SetWakeState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, state);
}
void SetReloadState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, state);
}
void SetResetState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_3, state);
}

void EnterLoraConfMode()
{
    SetResetState(RESET);
    DelaySendTask(10, ENTER_LORA_AT_MODEL); //拉低10ms Lora复位
}

void ATCMD_EnterLoraConfMode()
{
    SetResetState(SET);
    SendLora("+++", 3);
    SendLora("a", 1);
}