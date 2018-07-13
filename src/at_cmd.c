#include "lora.h"

void setLoraSerial()
{
}

void getLoraSerial()
{
}

void setLoraAddr(uint16_t addr)
{
}

void setLoraChan(uint8_t chan)
{
}

void initLora()
{
    GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //Host_WAKE，M_WAKE，M_Reload,M_RST
    GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Fast);                           //CTRL_RELAY
}

void setHostWakeState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, state);
}
void setMWakeState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, state);
}
void setMReloadState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, state);
}
void setMResetState(BitAction state)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_3, state);
}