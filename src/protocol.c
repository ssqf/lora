//协议的处理
//需要数据：1、sn；2、包序列号；3、ack确认；
#include "protocol.h"

#define BUFF_SIZE 100
static uint8_t Lorabuff[BUFF_SIZE];
static uint8_t Devbuff[BUFF_SIZE];
static uint8_t DevDataStartPos = 0;
static uint8_t DevDataEndPos = 0;
static uint8_t DevDataLen = 0;
static uint8_t DevRemainDataLen = BUFF_SIZE;
static uint8_t LoraDataStartPos = 0;
static uint8_t LoraDataEndPos = 0;
static uint8_t LoraDataLen = 0;
static uint8_t LoraRemainDataLen = BUFF_SIZE;

static bool isEnterLoraConfig(uint8_t *data, uint8_t len);

uint8_t GetBuffRemainLen()
{
    return LoraRemainDataLen;
}

bool PushDataLoraBuff(uint8_t *data, uint8_t len)
{
    uint8_t i = 0;

    if (len > LoraRemainDataLen)
    {
        return FALSE;
    }

    for (i = 0; LoraRemainDataLen > 0 && i < len; i++)
    {
        Lorabuff[LoraDataEndPos++] = *(data + i);
        LoraRemainDataLen--;
        if (LoraDataEndPos >= BUFF_SIZE)
        {
            LoraDataEndPos = 0;
        }
    }

    return TRUE;
}

bool PushDataDevBuff(uint8_t *data, uint8_t len)
{
    uint8_t i = 0;

    if (len > DevRemainDataLen)
    {
        return FALSE;
    }

    for (i = 0; DevRemainDataLen > 0 && i < len; i++)
    {
        Devbuff[DevDataEndPos++] = *(data + i);
        DevRemainDataLen--;
        if (DevDataEndPos >= BUFF_SIZE)
        {
            DevDataEndPos = 0;
        }
    }

    return TRUE;
}

void HandleLoraDate()
{
}

uint8_t SendLoraData[BUFF_SIZE];
uint8_t SendLoraDataLen = 0;
void HandleDevData()
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t len = BUFF_SIZE - DevRemainDataLen;
    for (i = 0, j = 0, k = DevDataStartPos; i < len; i++)
    {
        if (k >= BUFF_SIZE)
        {
            k = 0;
        }
        SendLoraData[j++] = Devbuff[k++];
    }
    DevDataStartPos = k;
    DevRemainDataLen += i;
    SendLoraDataLen = i;
    if (isEnterLoraConfig(SendLoraData, SendLoraDataLen))
    {
        EnterLoraConfMode();
    }
    //SendLora(SendLoraData, SendLoraDataLen);
    SetLoraReadySend();
}

uint8_t SendDevData[BUFF_SIZE];
uint8_t SendDevDataLen = 0;

void HandleLoraData()
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t len = BUFF_SIZE - LoraRemainDataLen;

    for (i = 0, j = 0, k = LoraDataStartPos; i < len; i++)
    {
        if (k >= BUFF_SIZE)
        {
            k = 0;
        }
        SendDevData[j++] = Lorabuff[k++];
    }
    LoraDataStartPos = k;
    LoraRemainDataLen += i;
    SendDevDataLen = i;

    SendDevice(SendDevData, SendDevDataLen);
    SendDevDataLen = 0;
}

void HandSendLoarData()
{
    if (0 == SendLoraDataLen)
    {
        return;
    }
    SendLora(SendLoraData, SendLoraDataLen);
    SendLoraDataLen = 0;
}

static bool isEnterLoraConfig(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        return FALSE;
    }
    if (strncmp(data, "+++", 3) == 0)
    {
        return TRUE;
    }
    return FALSE;
}