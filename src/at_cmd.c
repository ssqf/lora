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
    GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //Host_WAKE，M_WAKE，M_Reload,M_RST
    GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Fast);                           //CTRL_RELAY
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