//协议的处理
//需要数据：1、sn；2、包序列号；3、ack确认；
#include "protocol.h"

#define VER 1

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

uint8_t FRAME_NUM = 0;

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

//从buff中取得数据，如果isDel为true则将buff中数据删除
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

//处理设备收到的数据
void HandleDevData()
{
    HandSendLoarData();
    Debug("HandleDevData");
}

//处理lora收到的数据
void HandleLoraData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, TRUE);
    if (0 == desiredLen)
    {
        return;
    }
    SendDevice(data, desiredLen);
    Debug("HandleLoraData desiredLen:%d", desiredLen);
}

//处理要发送给lora的数据
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
帧头	    帧长	     帧号	     SN      版本	 数据类型	信道	 地址		data	    校验和	   帧尾
2byte	    1byte	    1byte	    4byte	1byte	1byte	  1byte	   1byte	 0-n	    1byte	   2byte
0x5a55	    0-0xFF	    0-0xFF		0-0xFF	0-0xFF	0-FF	  0-FF	   0-FF      0-0xFF	    0-0xFF    0x6A69


*/

typedef struct
{
    uint16_t Head;    //5A55
    uint8_t FrameLen; //帧长不包含帧头帧尾,但要包含数据内容的所有数据
    uint8_t FrameNum;
    uint32_t SerailNum;
    uint8_t Verison;
    uint8_t DataType;
    uint8_t ChanNum;
    uint16_t LoraAddr;
    uint8_t *Data;
    uint8_t SumCheck;
    uint16_t Tail; //6A69
} Frame;
Frame fra = {
    0x5A55,
    18,
    0,
    0,
    VER,
    0,
    72,
    0,
    NULL,
    0,
    0x6A69};
#define FrameMinLen (uint8_t)(sizeof(Frame) - sizeof(uint8_t *)) //帧最小长度，
#define FrameFixedLen (sizeof(Frame) - 4 - sizeof(uint8_t *))    // 帧固定长度 减去帧头帧尾和data的指针
uint8_t SumCheck(uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t sum = 0;
    for (i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

uint8_t FrameSumCheck(Frame f)
{
    uint8_t sum = 0;
    sum += f.FrameLen;
    sum += f.FrameNum;
    sum += f.SerailNum;
    sum += f.Verison;
    sum += f.DataType;
    sum += f.ChanNum;
    sum += f.LoraAddr;
    sum += SumCheck(f.Data, f.FrameLen - FrameFixedLen);
    return sum;
}

typedef enum
{
    FRAMEOK,
    DATALENNotENOUGH,
    SUMCHECKERROR,
} FramePraseRESULT;

uint16_t ToUint16(uint8_t *data)
{
    uint16_t u16 = data[1] < 8 | data[0];
    return u16;
}

uint32_t ToUint32(uint8_t *data)
{
    uint16_t u32 = data[3] < 24 | data[2] < 16 | data[1] < 8 | data[0];
    return u32;
}

//从数据量中解析出帧，注意必须要等该帧的数据用完之后再解析下一帧的数据，以免数据覆盖
//result 返回解析结果
//len 返回作废的数据
Frame PraseToFrame(uint8_t *data, uint8_t *len, FramePraseRESULT *result)
{
    uint8_t i;
    if (*len < FrameMinLen)
    {
        *result = DATALENNotENOUGH;
        return fra;
    }

    for (i = 0; i < *len - 1 && *len - i >= FrameMinLen; i++)
    {
        if (ToUint16(&data[i]) != fra.Head) //查找帧头
        {
            continue;
        }

        fra.FrameLen = data[i + 2]; //取得帧长

        if ((i + fra.FrameLen + 2) > *len) //数据是否够一帧长度
        {
            *result = DATALENNotENOUGH;
            *len = i;
            return fra;
        }

        if (ToUint16(&data[i] + fra.FrameLen) != 0x6A69) //帧尾不能匹配
        {
            *result = DATALENNotENOUGH;
            *len = i + fra.FrameLen + 2; //数据全部作废
            return fra;
        }

        if (SumCheck(&data[i + 2], fra.FrameLen - 1) != data[i + 2 + fra.FrameLen])
        {
            *result = SUMCHECKERROR;
            *len = i + fra.FrameLen + 2; //数据全部作废
            return fra;
        }

        fra.FrameNum = data[i + 3];
        fra.SerailNum = ToUint32(&data[i + 4]);
        fra.Verison = data[i + 8];
        fra.DataType = data[i + 9];
        fra.ChanNum = data[i + 10];
        fra.LoraAddr = ToUint16(&data[i + 11]);
        fra.Data = &data[i + 13];
        fra.SumCheck = data[i + fra.FrameLen - 1];

        *result = FRAMEOK;
        *len = i + fra.FrameLen + 2;
        return fra;
    }

    *result = DATALENNotENOUGH;
    *len = 0; //数据全部作废
    return fra;
}

// 用数据创建帧
Frame newFrame = {
    0x5A55,
    18,
    0,
    0,
    VER,
    0,
    72,
    0,
    NULL,
    0,
    0x6A69};

Frame CreateFrame(uint8_t *data, uint8_t len)
{
    newFrame.Data = data;
    newFrame.FrameLen = FrameFixedLen + len;
    newFrame.ChanNum = conf.LoraChan;
    newFrame.LoraAddr = conf.LoraAddr;
    newFrame.DataType = conf.DevType;
    newFrame.SerailNum = conf.SN;
    newFrame.FrameNum = FRAME_NUM++;
    newFrame.Verison = VER;
    newFrame.SumCheck = FrameSumCheck(newFrame);
    return newFrame;
}

uint8_t GetFrameLen(Frame f)
{
    return f.FrameLen + 4; //加上帧头帧尾
}

void WriteFrameToBuff(uint8_t *buff, Frame f)
{
    uint8_t len = 0;
    buff[0] = f.Head >> 8;
    buff[1] = (uint8_t)(f.Head | 0xFF);
    buff[2] = f.FrameLen;
    buff[3] = f.FrameNum;
    buff[4] = f.SerailNum >> 24;
    buff[5] = f.SerailNum >> 16;
    buff[6] = f.SerailNum >> 8;
    buff[7] = (uint8_t)(f.SerailNum & 0xFF);
    buff[8] = f.Verison;
    buff[9] = f.DataType;
    buff[10] = f.ChanNum;
    buff[11] = f.LoraAddr >> 8;
    buff[12] = f.LoraAddr & 0xFF;
    len = f.FrameLen - FrameFixedLen;
    memcpy(&buff[13], f.Data, len);
    buff[13 + len] = f.SumCheck;
    buff[13 + len + 1] = f.Tail >> 8;
    buff[13 + len + 2] = f.Tail & 0xFF;
}