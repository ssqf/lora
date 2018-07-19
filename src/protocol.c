//协议的处理
//需要数据：1、sn；2、包序列号；3、ack确认；
#include "protocol.h"

#define BUFF_SIZE 200
static uint8_t Lorabuff[BUFF_SIZE];
static uint8_t Devbuff[BUFF_SIZE];
static uint8_t DevDataStartPos = 0;
static uint8_t DevDataEndPos = 0;
//static uint8_t DevDataLen = 0;
static uint8_t DevRemainDataLen = BUFF_SIZE;
static uint8_t LoraDataStartPos = 0;
static uint8_t LoraDataEndPos = 0;
//static uint8_t LoraDataLen = 0;
static uint8_t LoraRemainDataLen = BUFF_SIZE;
uint8_t LoraData[BUFF_SIZE];
uint8_t DevData[BUFF_SIZE];

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

void ClearDevBuff(uint8_t len)
{
    if (len == 0)
    {
        DevDataStartPos = 0;
        DevDataEndPos = 0;
        DevRemainDataLen = BUFF_SIZE;
    }
    else
    {
        if (BUFF_SIZE - DevDataStartPos < len)
        {
            DevDataStartPos = len - (BUFF_SIZE - DevDataStartPos);
        }
        else
        {
            DevDataStartPos += len;
        }

        DevRemainDataLen += len;
    }
}

void ClearLoraBuff(uint8_t len)
{
    if (len == 0)
    {
        LoraDataStartPos = 0;
        LoraDataEndPos = 0;
        LoraRemainDataLen = BUFF_SIZE;
    }
    else
    {
        if (BUFF_SIZE - LoraDataStartPos < len)
        {
            LoraDataStartPos = len - (BUFF_SIZE - LoraDataStartPos);
        }
        else
        {
            LoraDataStartPos += len;
        }

        LoraRemainDataLen += len;
    }
}

uint8_t *GetDevDataFromBuff(uint8_t *desiredLen, bool isDel)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t totalLen = BUFF_SIZE - DevRemainDataLen;
    uint8_t len = 0;
    if (*desiredLen == 0) //desiredLen为空获取所有数据
    {
        len = totalLen;
    }
    else
    {
        len = totalLen <= *desiredLen ? totalLen : *desiredLen;
    }
    for (i = 0, j = 0, k = DevDataStartPos; i < len; i++)
    {
        if (k >= BUFF_SIZE)
        {
            k = 0;
        }
        DevData[j++] = Devbuff[k++];
    }
    if (isDel)
    {
        DevDataStartPos = k;
        DevRemainDataLen += i;
    }

    *desiredLen = i;
    Debug("GetDevDataFromBuff totalLen:%d,desiredLen:%d", totalLen, i);
    return DevData;
}

//isDel边上获取后是否从buff中删除
uint8_t *GetLoraDataFromBuff(uint8_t *desiredLen, bool isDel)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t totalLen = BUFF_SIZE - LoraRemainDataLen;
    uint8_t len = 0;
    if (*desiredLen == 0) //desiredLen为空获取所有数据
    {
        len = totalLen;
    }
    else
    {
        len = totalLen <= *desiredLen ? totalLen : *desiredLen;
    }
    for (i = 0, j = 0, k = LoraDataStartPos; i < len; i++)
    {
        if (k >= BUFF_SIZE)
        {
            k = 0;
        }
        LoraData[j++] = Lorabuff[k++];
    }
    if (isDel)
    {
        LoraDataStartPos = k;
        LoraRemainDataLen += i;
    }
    *desiredLen = i;
    Debug("GetLoraDataFromBuff totalLen:%d,desiredLen:%d", totalLen, i);
    return LoraData;
}

void HandleDevData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, FALSE);
    if ((AT_Status == LORA_TRANSFER) && IsEnterLoraConfig(data, desiredLen)) //判断是不是+++ 要进入AT模式
    {
        ClearDevBuff(desiredLen);
        ATCMD_RestartBegin();
        DelaySendTask(3000, ENTER_ATMODLE_TIMEOUT); //3s没有进AT模式则超时
        AT_Status = LORA_RESTART;
        Debug("RestartBegin LORA_RESTART");
    }

    if (AT_Status == LORA_IN_ATCMD)
    {
        if (IsRedayExitATCmd(data, desiredLen)) //判断是不是"AT+ENTM\r\n" 要退出AT模式
        {
            AT_Status = LORA_READY_EXIT_ATCMD;
            //SetLoraReadySend();
            DelaySendTask(10, EXIT_AT_TIMEOUT);
            HandSendLoarData();
            Debug("LORA_READY_EXIT_ATCMD");
        }
    }

    //SendLora(SendLoraData, SendLoraDataLen);
    if ((AT_Status == LORA_TRANSFER) || (AT_Status == LORA_IN_ATCMD))
    {
        //SetLoraReadySend();
        HandSendLoarData();
    }
}

void HandleLoraData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, TRUE);
    SendDevice(data, desiredLen);
}

void HandSendLoarData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, TRUE);
    if (0 == desiredLen)
    {
        return;
    }
    SendLora(data, desiredLen);
    Debug("HandSendLoarData desiredLen:%d", desiredLen);
}



/*
协议说明

*/