//协议的处理
//需要数据：1、sn；2、包序列号；3、ack确认；
#include "lora.h"
void HandleDeviceData(uint8_t *data, uint8_t len)
{
}

#define BUFF_SIZE 100
static uint8_t Lorabuff[BUFF_SIZE];
static uint8_t Devbuff[BUFF_SIZE];
static uint8_t dataStartPos = 0;
static uint8_t dataEndPos = 0;
static uint8_t dataLen = 0;
static uint8_t remainDataLen = BUFF_SIZE;

uint8_t GetBuffRemainLen()
{
    return remainDataLen;
}

bool PushData(uint8_t *data, uint8_t len)
{
    uint8_t i = 0;

    if (len > remainDataLen)
    {
        return FALSE;
    }

    for (i = 0; remainDataLen > 0 && i < len; i++)
    {
        Lorabuff[dataEndPos++] = *(data + i);
        remainDataLen--;
        if (dataEndPos >= BUFF_SIZE)
        {
            dataEndPos = 0;
        }
    }

    return TRUE;
}

void HandleLoraData()
{
}

void HandleDevData()
{
}