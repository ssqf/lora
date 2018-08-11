//协议的处理
//需要数据：1、sn；2、包序列号；3、ack确认；
//一帧最长100个字节

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
static bool isSyncTime = FALSE;

uint8_t LoraData[BUFF_SIZE];
uint8_t DevData[BUFF_SIZE];

#define Frame_Buff_Size 100
#define Frame_Data_Max_Len (Frame_Buff_Size - FrameFixedLen)
uint8_t FrameBuff[Frame_Buff_Size];
uint8_t FrameDataBuff[Frame_Buff_Size];

uint8_t FRAME_NUM = 0;

// 用数据创建帧
//Frame newFrame = {0x5A55, 18, 0, 0, VER, 0, 0, 72, 0, NULL, 0, 0x6A69};
//Frame fra = {0x5A55, 18, 0, 0, VER, 0, 0, 72, 0, NULL, 0, 0x6A69};
//协议发送的要收到ACK，才能进行下次一次发送，一直超时，则认为是网络连接断开
bool LoraIsAck = TRUE;
bool DevIsAck = TRUE;

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
    Debug("GetDevDataFromBuff totalLen:%u,desiredLen:%u", totalLen, i);
    return DevData;
}

//isDel确定获取后是否从buff中删除
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
    Debug("GetLoraDataFromBuff totalLen:%u,desiredLen:%u", totalLen, i);
    return LoraData;
}

//处理设备串口收到的数据
void HandleDevData()
{
    if (Conf.DevType == DevType_GW) //网关的Dev收到数据是Serv发送过来的,要么自己处理，要么通过Lora转发给节点
    {
        HandleServData();
        Debug("HandleServData");
    }
    else if (Conf.DevType == DevType_LoRa_Node) //Lora节点收到Dev数据是设备发送过来的,组装成帧发送给Serv
    {
        if (DevStatus != DEV_DATA_TRANSFER)
        {
            Debug("HandleDevData DevStatus != DEV_DATA_TRANSFER Clear All data");
            ClearDevBuff(0);
            return;
        }
        if (!isFitter())
        {
            DevDataByFrameSendLoar();
            Debug("DevDataByFrameSendLoar");
        }
        else
        {
            DibangDataHandler();
        }
    }
    else
    {
        Debug("HandleDevData Error");
    }
}

//处理Lora串口收到的数据
void HandleLoraData()
{
    if (Conf.DevType == DevType_GW) //网关的Lora收到数据是要发送给Serv，直接转发
    {
        LoraDataTransfToDev();
        Debug("LoraDataTransfToDev");
    }
    else if (Conf.DevType == DevType_LoRa_Node) //Lora节点收到Lora数据,是网关发送过来的，解析命令处理，或者解析出数据发送给设备
    {
        HandleLoraNodeData();
        Debug("HandleLoraNodeData");
    }
    else
    {
        Debug("HandleLoraData Error");
    }
}

// GPRS 网关处理来自服务器的数据
void HandleServData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, FALSE);
    if (0 == desiredLen)
    {
        return;
    }

    FramePraseRESULT result;
    Frame f = PraseToFrame(data, &desiredLen, &result);
    if (result == DATALEN_NotENOUGH)
    {
        Debug("HandleServData PraseToFrame DATALEN_NotENOUGH");
        return;
    }
    else if (result == FRAME_ERROR || result == SUMCHECK_ERROR)
    {
        ClearLoraBuff(desiredLen);
        Debug("HandleServData PraseToFrame FRAME_ERROR");
        return;
    }
    else if (result == FRAME_OK)
    {
        ClearLoraBuff(desiredLen);
        if (f.DataType == Who_Ara_You) //查询是那个网关
        {
            GwRespServ();
            Debug("GwRespServ");
        }
        else if (f.DataType == SYNC_TIME)
        {
            if (f.SerailNum == Conf.SN)
            {
                SyncTimeToLoacl(*(uint32_t *)f.Data, 0, 0);
                isSyncTime = TRUE;
            }
            else if (isSyncTime) //网关要先同步过时间
            {
                *(uint32_t *)f.Data = Second;
                f.Data[3] = TickNum;
                *(uint16_t *)&f.Data[4] = TIM2_GetCounter();
                SendDataToLoraNode(f);
            }
        }
        else if (f.DataType == I_AM_GW) //响应了注册信息
        {
        }
        else
        {
            SendDataToLoraNode(f);
        }
        return;
    }
    else
    {
        Debug("HandleServData PraseToFrame Other Error");
    }
}

//发现设备是个网关
void DeviceIsGateway()
{
    if (isGPRS_GW) //已经有GPRS响应过了，则由GPRS流程完成配置
    {
        return;
    }

    if (Conf.DevType == DevType_UnKnown) //是一个未知的设备，收到Who_Ara_You，或者I_AM_GW的注册确认消息，则说明通过Dev串口可以连接的服务器，该设备可以作为网关
    {
        AbortGPRSAtCmd();
        Conf.LoraMode = 1;
        Conf.SendSlotTime = 2;
        EEPROM_Write_8(loraModeAddr, 1);
        EEPROM_Write_8(SendSlotTimeAddr, 2);
        SetConfFlag(Conf_FLAG_GPRS); //既然已经可以连接服务器，则不再需要配置GPRS的参数，认为它配置完成了
        SetDevType(DevType_GW);

        PushTask(CONF_LORA_PARM); //设备类型确认为网关，启动Lora参数配置
    }
}

//网关收到数据要发送给Lora节点
void SendDataToLoraNode(Frame f)
{
    if (LoraStatus != LORA_TRANSFER)
    {
        Debug("SendDataToLoraNode DevStatus != DEV_DATA_TRANSFER");
        return;
    }
    //网关是顶点模式，节点是透传模式
    FrameBuff[0] = f.LoraAddr >> 8;
    FrameBuff[0] = (uint8_t)f.LoraAddr;
    FrameBuff[0] = f.LoraChanNum;
    WriteFrameToBuff(FrameBuff + 3, f);
    SendLora(FrameBuff, GetFrameLen(f));
}

void GwRespServ()
{
    Frame f = CreateFrame(I_AM_GW, NULL, 0);
    WriteFrameToBuff(FrameBuff, f);
    SendDevice(FrameBuff, GetFrameLen(f));
}

//处理lora收到的数据
void HandleLoraNodeData()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, FALSE);
    if (0 == desiredLen)
    {
        return;
    }
    FramePraseRESULT result;
    Frame f = PraseToFrame(data, &desiredLen, &result);
    if (result == DATALEN_NotENOUGH)
    {
        Debug("HandleLoraData PraseToFrame DATALEN_NotENOUGH");
        return;
    }
    else if (result == FRAME_ERROR || result == SUMCHECK_ERROR)
    {
        ClearLoraBuff(desiredLen);
        Debug("HandleLoraData PraseToFrame FRAME_ERROR");
        return;
    }
    else if (result == FRAME_OK)
    {
        ClearLoraBuff(desiredLen);
        if (f.SerailNum == Conf.SN)
        {
            switch (f.DataType)
            {
            case LORA_DATA_TRANSF: //透传直接转发给设备
                SendDevice(f.Data, GetFrameDataLen(f));
                break;
            case I_AM_LORA_NODE:
                LoraRegisterDone(f);
                break;
            case NODE_CONN_WHICH_DEVICE:
                GetLoraDevConfDone(f);
                break;
            case SYNC_TIME:
                SyncTimeToLoacl(*((uint32_t *)(&f.Data)), f.Data[4], *((uint16_t *)(&f.Data)));
                isSyncTime = TRUE;
                break;
            case HEARTBEAT:
                //心跳只发不接受Ack
                break;
            case DEVCMD:
                ConfDevCmd(f);
                break;
            case DEVFITTER:
                ConfDevFitter(f);
                break;
            }
        }
        else
        {
            Debug("HandleLoraData PraseToFrame Not Our Frame");
        }
        return;
    }
    else
    {
        Debug("HandleLoraData PraseToFrame Other Error");
    }

    //SendDevice(data, desiredLen);
    Debug("HandleLoraData desiredLen:%u", desiredLen);
}

//lora口收到的数据直接转发到Dev口发送出去，连接GPRS则发送服务器
void LoraDataTransfToDev()
{
    if (DevStatus != DEV_DATA_TRANSFER)
    {
        ClearLoraBuff(0);
        Debug("LoraDataTransfToDev LoraStatus != LORA_TRANSFER,And Clear Lora Buff");
        return;
    }
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, TRUE);
    if (0 == desiredLen)
    {
        return;
    }
    SendDevice(data, desiredLen);
    Debug("LoraDataTransfToDev desiredLen:%u", desiredLen);
}

//dev口收到的数据直接转发给lora口发送出去
void DevDataTransfLora()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, TRUE);
    if (0 == desiredLen)
    {
        return;
    }
    SendLora(data, desiredLen);
    Debug("DevDataTransfLora desiredLen:%u", desiredLen);
}

//将dev的数据组成帧发送服务器
void DevDataByFrameSendLoar()
{
    uint8_t desiredLen;
    desiredLen = Frame_Data_Max_Len;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, TRUE);
    if (0 == desiredLen)
    {
        return;
    }

    if (DevStatus != DEV_DATA_TRANSFER)
    {
        Debug("DevDataByFrameSendLoar DevStatus != DEV_DATA_TRANSFER");
        return;
    }

    Frame f = CreateFrame(LORA_DATA_TRANSF, data, desiredLen);
    WriteFrameToBuff(FrameBuff, f);
    SendLora(FrameBuff, GetFrameLen(f));
    Debug("DevDataSendLoar desiredLen:%u", desiredLen);
}

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

//不包含帧头帧尾
uint8_t FrameSumCheck(Frame f)
{
    uint8_t sum = 0;
    sum += f.FrameLen;
    sum += f.FrameNum;
    sum += f.SerailNum;
    sum += f.Verison;
    sum += f.DataType;
    sum += f.LoraChanNum;
    sum += f.LoraAddr;
    sum += SumCheck(f.Data, f.FrameLen - FrameFixedLen);
    return sum;
}

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
//len 传入数据的长度,返回作废的数据长度
Frame PraseToFrame(uint8_t *data, uint8_t *len, FramePraseRESULT *result)
{
    uint8_t i;
    Frame fra = {0};
    if (*len < FrameMinLen)
    {
        *result = DATALEN_NotENOUGH;
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
            *result = DATALEN_NotENOUGH;
            *len = i;
            return fra;
        }

        if (ToUint16(&data[i] + fra.FrameLen) != 0x6A69) //帧尾不能匹配,继续查找合适的帧
        {
            //*result = DATALEN_NotENOUGH;
            //*len = i + fra.FrameLen + 2; //数据全部作废
            //return fra;
            continue;
        }

        if (SumCheck(&data[i + 2], fra.FrameLen - 1) != data[i + 2 + fra.FrameLen])
        {
            *result = SUMCHECK_ERROR;
            *len = i + fra.FrameLen + 2; //数据全部作废
            return fra;
        }

        fra.FrameNum = data[i + 3];
        fra.SerailNum = ToUint32(&data[i + 4]);
        fra.Verison = data[i + 8];
        fra.DataType = data[i + 9];
        fra.ConnDevType = data[i + 10];
        fra.LoraChanNum = data[i + 11];
        fra.LoraAddr = ToUint16(&data[i + 12]);
        uint8_t dataLen = GetFrameDataLen(fra);
        if (dataLen > Frame_Buff_Size)
        {
            Debug("data len > Frame_Buff_Size");
            *result = FRAME_ERROR;
            *len = i + fra.FrameLen + 2;
            return fra;
        }
        else
        {
            fra.Data = FrameDataBuff;
            memcpy(fra.Data, &data[i + 14], dataLen);
        }
        //fra.Data = &data[i + 14];
        fra.SumCheck = data[i + fra.FrameLen - 1];

        *result = FRAME_OK;
        *len = i + fra.FrameLen + 2;
        return fra;
    }

    *result = DATALEN_NotENOUGH;
    *len = 0; //数据全部作废
    return fra;
}

//创建帧
Frame CreateFrame(FrameDataType dataType, uint8_t *data, uint8_t len)
{
    Frame newFrame;
    newFrame.Head = 0x5A55;
    newFrame.Tail = 0x6A69;

    newFrame.Data = data;
    newFrame.FrameLen = FrameFixedLen + len;
    newFrame.LoraChanNum = Conf.LoraChan;
    newFrame.LoraAddr = Conf.LoraAddr;
    newFrame.DataType = dataType;
    newFrame.ConnDevType = Conf.ConnDevType;
    newFrame.SerailNum = Conf.SN;
    newFrame.FrameNum = ++FRAME_NUM;
    newFrame.Verison = VER;
    newFrame.SumCheck = FrameSumCheck(newFrame);
    return newFrame;
}

uint8_t GetFrameLen(Frame f)
{
    return f.FrameLen + 4; //加上帧头帧尾
}

uint8_t GetFrameDataLen(Frame f)
{
    return f.FrameLen + 4 - FrameFixedLen;
}

void WriteFrameToBuff(uint8_t *buff, Frame f)
{
    uint8_t len = 0;
    buff[0] = f.Head >> 8;
    buff[1] = (uint8_t)(f.Head & 0xFF);
    buff[2] = f.FrameLen;
    buff[3] = f.FrameNum;
    buff[4] = f.SerailNum >> 24;
    buff[5] = (f.SerailNum >> 16) & 0xFF;
    buff[6] = (f.SerailNum >> 8) & 0xFF;
    buff[7] = (uint8_t)(f.SerailNum & 0xFF);
    buff[8] = f.Verison;
    buff[9] = f.DataType;
    buff[10] = f.ConnDevType;
    buff[11] = f.LoraChanNum;
    buff[12] = f.LoraAddr >> 8;
    buff[13] = f.LoraAddr & 0xFF;
    len = f.FrameLen - FrameFixedLen;
    memcpy(&buff[13], f.Data, len);
    buff[14 + len] = f.SumCheck;
    buff[14 + len + 1] = f.Tail >> 8;
    buff[14 + len + 2] = f.Tail & 0xFF;
}

//发送网关注册
void GwRegister()
{
    static uint8_t tryTimes = 0;
    if (Conf.DevType == DevType_UnKnown)
    {
        if (tryTimes < 3)
        {
            DevStatus = GW_REGISTER;
            tryTimes++;
            Frame f = CreateFrame(I_AM_GW, NULL, 0);
            uint8_t fLen = GetFrameLen(f);
            if (fLen < Frame_Buff_Size)
            {
                WriteFrameToBuff(FrameBuff, f);
                SendDevice(FrameBuff, fLen);
            }
            else
            {
                Debug("Send GwRegister error");
            }
            DelaySendTask(ReqConfPeriod / 2, GW_Register_Task);
        }
        else //注册三次也不成功则认为是Lora节点
        {
            SetDevType(DevType_LoRa_Node);
            PushTask(CONF_LORA_PARM);
            Debug("PushTask(CONF_LORA_PARM)");
        }
    }
}

void LoraRegister()
{
    if (RESET == GetConfFlag(Conf_FLAG_LORA_REGIST))
    {
        LoraStatus = LORA_REGISTER;
        Frame f = CreateFrame(I_AM_LORA_NODE, NULL, 0);
        uint8_t fLen = GetFrameLen(f);
        if (fLen < Frame_Buff_Size)
        {
            WriteFrameToBuff(FrameBuff, f);
            SendLora(FrameBuff, fLen);
        }
        else
        {
            Debug("Send LoraRegister error");
        }
        DelaySendTask(ReqConfPeriod, LoraRegister_Task);
    }
}

/*
lora节点注册
回复中应该包含 Lora地址，Lora通道，在帧中，Lora的模式，Lora的空传速率，在数据字段中
*/
void LoraRegisterDone(Frame f)
{
    Conf.LoraAddr = f.LoraAddr;
    Conf.LoraChan = f.LoraChanNum;
    Conf.LoraMode = f.Data[0];
    Conf.LoraAirRate = f.Data[1];
    EEPROM_Write_BUFF(loraAddr, (uint8_t *)&Conf.LoraAddr, 5);

    SetConfFlag(Conf_FLAG_LORA_REGIST);
    LoraStatus = LORA_TRANSFER;
    InitLoraConf();
}

void GetLoraDevConf()
{
    if (RESET == GetConfFlag(Conf_FLAG_LORA_DEV_Done))
    {
        LoraStatus = GET_LORA_DEV_PARAM;
        Frame f = CreateFrame(NODE_CONN_WHICH_DEVICE, NULL, 0);
        uint8_t fLen = GetFrameLen(f);
        if (fLen < Frame_Buff_Size)
        {
            WriteFrameToBuff(FrameBuff, f);
            SendLora(FrameBuff, fLen);
        }
        else
        {
            Debug("Send GetLoraDevConf error");
        }
        DelaySendTask(ReqConfPeriod, GetLoraDevParam_Task);
    }
}

// 设备注册请求相关设备参数
// 所连接的设备类型，串口参数，发送时隙，心跳时隙,在Data字段
void GetLoraDevConfDone(Frame f)
{
    Conf.ConnDevType = f.Data[0];
    Conf.DevBaudRate = f.Data[1];
    Conf.DevDataBit = f.Data[2];
    Conf.DevStopBit = f.Data[3];
    Conf.DevParityBit = f.Data[4];
    Conf.SendSlotTime = f.Data[5];
    Conf.HeartBeatTime = f.Data[6];

    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    Conf.ConfFlag &= ~Conf_FLAG_INTEGRITY; //清除完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_ProgramByte(connDevTypeAddr, Conf.ConnDevType);
    FLASH_ProgramByte(DevBaudRateAddr, Conf.DevBaudRate);
    FLASH_ProgramByte(DevDataBitAddr, Conf.DevDataBit);
    FLASH_ProgramByte(DevStopBitAddr, Conf.DevStopBit);
    FLASH_ProgramByte(DevParityAddr, Conf.DevParityBit);
    FLASH_ProgramByte(SendSlotTimeAddr, Conf.SendSlotTime);
    FLASH_ProgramByte(HeartBeatTimeAddr, Conf.HeartBeatTime);

    Conf.ConfFlag |= Conf_FLAG_LORA_DEV;
    FLASH_ProgramByte(flagAddr + 2, (uint8_t)(Conf.ConfFlag >> 8));
    Conf.ConfFlag |= Conf_FLAG_INTEGRITY; //设置完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
    SetConfFlag(Conf_FLAG_LORA_DEV_Done);
    LoraStatus = LORA_TRANSFER;
    ReConfDeviceUart(); //重启配置device串口的参数
    Debug("GetLoraDevConfDone");
}

//DEVCMD
//0，第几条命令; 1,命令数据的长度;2,3，发送的周期;4-n,实际命令数据
void ConfDevCmd(Frame f)
{
    Conf.cmdLen[f.Data[0]] = f.Data[1];
    Conf.cmdPeriod[f.Data[0]] = ((uint16_t)f.Data[2] << 8) | (uint16_t)f.Data[3];
    //写入EEPROM
    FLASH_Write_Buff((CmdFlashAddr + ((uint32_t)f.Data[0] * (uint32_t)FLASH_BLOCK_SIZE)), &f.Data[4], f.Data[1]);

    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    Conf.ConfFlag &= ~Conf_FLAG_INTEGRITY; //清除完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_ProgramByte(CmdLenAddr + sizeof(Conf.cmdLen) * f.Data[0], f.Data[1]);
    FLASH_ProgramByte(CmdPeriodAddr + sizeof(Conf.cmdPeriod) * f.Data[0], f.Data[2]);
    FLASH_ProgramByte(CmdPeriodAddr + sizeof(Conf.cmdPeriod) * f.Data[0] + 1, f.Data[3]);

    Conf.ConfFlag |= Conf_FLAG_INTEGRITY; //设置完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
    LoraRespAck(f);
}

//0，第几条命令; 1,命令数据的长度;2,过滤方式；3，4过滤参数;5-n,实际命令数据
void ConfDevFitter(Frame f)
{
    Conf.fitterLen[f.Data[0]] = f.Data[1];
    Conf.fitterType[f.Data[0]] = f.Data[2];
    Conf.fitterParm[f.Data[0]] = ((uint16_t)f.Data[3] << 8) | (uint16_t)f.Data[4];

    FLASH_Write_Buff((FitterFlashAddr + ((uint32_t)f.Data[0] * (uint32_t)FLASH_BLOCK_SIZE)), &f.Data[5], f.Data[1]);

    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    Conf.ConfFlag &= ~Conf_FLAG_INTEGRITY; //清除完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_ProgramByte(FitterLenAddr + sizeof(Conf.fitterLen) * f.Data[0], f.Data[1]);
    FLASH_ProgramByte(FitterTypeAddr + sizeof(Conf.fitterType) * f.Data[0], f.Data[2]);
    FLASH_ProgramByte(FitterParmAddr + sizeof(Conf.fitterParm) * f.Data[0], f.Data[3]);
    FLASH_ProgramByte(FitterParmAddr + sizeof(Conf.fitterParm) * f.Data[0] + 1, f.Data[4]);

    Conf.ConfFlag |= Conf_FLAG_INTEGRITY; //设置完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);

    LoraRespAck(f);
}

void LoraRespAck(Frame f)
{
    uint8_t dataLen = GetFrameDataLen(f);
    f.FrameLen -= dataLen;
    f.Data = NULL;
    f.DataType = ACK;
    WriteFrameToBuff(FrameBuff, f);
    SendLora(FrameBuff, GetFrameLen(f));
}

void SendHeartBeat()
{
    if (isSyncTime == FALSE) //时间完成同步才可以发送心跳
    {
        return;
    }

    if (Conf.DevType == 1) //网关发送给Dev串口
    {
        Frame f = CreateFrame(HEARTBEAT, NULL, 0);
        WriteFrameToBuff(FrameBuff, f);
        SendDevice(FrameBuff, GetFrameLen(f));
    }
    else if (Conf.DevType == 2) //节点发送到Lora串口
    {
        Frame f = CreateFrame(SYNC_TIME, NULL, 0);
        WriteFrameToBuff(FrameBuff, f);
        SendLora(FrameBuff, GetFrameLen(f));
    }
}

void SendSyncTime()
{
    if (!isSyncTime)
    {
        if (Conf.DevType == 1)
        {
            Frame f = CreateFrame(SYNC_TIME, NULL, 0);
            WriteFrameToBuff(FrameBuff, f);
            SendDevice(FrameBuff, GetFrameLen(f));
        }
        else if (Conf.DevType == 2)
        {
            Frame f = CreateFrame(SYNC_TIME, NULL, 0);
            WriteFrameToBuff(FrameBuff, f);
            SendLora(FrameBuff, GetFrameLen(f));
        }
        else
        {
            Debug("DevType Unkonw Sync don't send");
        }

        DelaySendTask(ReqConfPeriod, SyncTime_Task); //10s尝试同步一次
    }
}

bool isFitter()
{
    for (uint8_t i = 0; i < FITTER_NUMBER; i++)
    {
        if (Conf.fitterLen[i] != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// typedef enum
// {
//     Match_OK,              //匹配正确
//     Match_Data_Not_Enough, //数据不够
//     Match_Err,             //直接不匹配
// } PatternResult;
//16进制模式匹配，模仿简单的正则，A-F全为大写
// *任意内容匹配；+至少匹配前面字节内容一次；.匹配0-n次；{n}固定匹配n次
//

//PatternResult PatternMatch(char const *pattern, uint8_t *data, uint8_t dataLen, uint8_t matchcStart, uint8_t matchEnd)
//{
//    while (*pattern)
//}

//过滤地磅连续发生数据
bool DiBangFitter(uint8_t const *pattern, uint8_t patternLen, uint8_t *data, uint8_t dataLen, uint8_t *matchcStart, uint8_t *matchEnd)
{
    //找到数据的开始
    uint8_t i = 0;
    while (data[i] != pattern[0])
    {
        i++;
        if (i >= dataLen)
        {
            *matchcStart = i;
            *matchEnd = i;
            return FALSE;
        }
    }

    if (dataLen - i < patternLen)
    {
        *matchcStart = i;
        *matchEnd = i;
        return FALSE;
    }
    data += i;
    *matchcStart += i;
    *matchEnd += i;
    for (i = 0; i < patternLen; i++)
    {
        if (data[i] != pattern[i] && data[i] != 0xFF)
        {
            *matchEnd += i;
            return FALSE;
        }
    }
    *matchEnd += i;
    return TRUE;
}

void DibangDataHandler()
{
    //static bool isContinuous = FALSE;
    static bool isSendData = FALSE;
    static uint8_t lastData[50];
    static uint8_t continuousNum = 0;

    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, FALSE);

    if (desiredLen < Conf.fitterLen[0]) //数据不够
    {
        return;
    }

    uint8_t matchcStart;
    uint8_t matchEnd;
    bool result = DiBangFitter((uint8_t *)FitterFlashAddr, Conf.fitterLen[0], data, desiredLen, &matchcStart, &matchEnd);

    if (result == TRUE)
    {
        ClearDevBuff(matchEnd);
        if (memcmp(lastData, data + matchcStart, Conf.fitterLen[0]) == 0)
        {
            if (continuousNum < 0xFF)
            {
                continuousNum++;
            }
            //isContinuous = TRUE;
        }
        else
        {
            memcpy(lastData, data + matchcStart, Conf.fitterLen[0]);
            continuousNum = 0;
            //isContinuous = FALSE;
            isSendData = FALSE;
        }
    }
    else
    {
        Debug("Find Fitter pattern data faild");
        ClearDevBuff(matchcStart);
    }

    if (!isSendData && continuousNum > Conf.fitterParm[0])
    {
        Frame f = CreateFrame(LORA_DATA_TRANSF, lastData, Conf.fitterLen[0]);
        WriteFrameToBuff(FrameBuff, f);
        SendLora(FrameBuff, GetFrameLen(f));
        Debug("DibangDataHandler desiredLen:%u", desiredLen);
        isSendData = TRUE;
    }
}
