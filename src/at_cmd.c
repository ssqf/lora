#include "lora.h"

ATCmd_Status Lora_AT_Status = LORA_EXIT_ATCMD;
GPRS_ATCmd_Status GPRS_AT_Status = GPRS_EXIT_ATCMD;
bool isGPRS_GW = FALSE; //是不是已经去人是GPRS网关
uint8_t EnterGPRS_AtTryTimes = 0;
GPRS_CMD_NAME GPRS_Curr_Cmd = SET_GPRS_UART_CMD;
LORA_CMD_NAME CurrLoraCmd = SET_LORA_UART_CMD;

void InitGPRSConf()
{
    Debug("InitGPRSConf");
    ReadyEnterGPRS_AT();
}

void InitLoraConf()
{
    Debug("InitLoraConf");
    ATCMD_RestartBegin();
}

#define LORACMDNUM (sizeof(LORACMD)) / sizeof(LORACMD[0])
uint8_t *LORACMD[] = {
    "AT+UART=115200,8,1,NONE,NFC\r\n",
    "AT+WMODE=%s\r\n",
    "AT+ADDR=%u\r\n",
    "AT+CH=%u",
    "AT+SPD=%u",
    "AT+CFGTF\r\n",
    "AT+ENTM\r\n",
    "",
    "\r\nOK\r\n",
};

void InitLoraPin()
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

//进AT命令模式步骤
//1、复位LoRa模组
//2、复位后2秒内给LoRa发送+++
//3、收到LoRa返回的a，再给LoRa发送a
//4、收到LoRa返回的+OK表示进入AT 命令模式
void ATCMD_RestartBegin()
{
    SetResetState(RESET);
    DelaySendTask(20, ATCMD_RESTARTEND); //拉低20ms Lora复位
    Lora_AT_Status = LORA_RESTART;
    LoraStatus = ENTER_LORA_AT_CMD;
    DelaySendTask(ReqConfPeriod, ENTER_ATMODLE_TIMEOUT);
}

void ATCMD_ResartEnd()
{
    SetResetState(SET);
}

bool IsLoraStart(uint8_t *data, uint8_t len)
{
    char *str = "LoRa Start!\r\n";
    if (len < strlen(str))
    {
        return FALSE;
    }
    if (strstr((char const *)data, str) != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

bool IsEnterATCmdResp(uint8_t *data, uint8_t len)
{
    if (len < 1)
    {
        return FALSE;
    }
    if (strstr((char const *)data, "a") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

bool IsEnterATCmdOK(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        return FALSE;
    }
    if (strstr((char const *)strupr(data, len), "+OK") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

bool IsLoraATCmdOK(uint8_t *data, uint8_t len)
{
    if (len < 8)
    {
        return FALSE;
    }
    if (strstr((char const *)strupr(data, len), "\r\nOK\r\n") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

void EnterAtModelTimeout()
{
    Debug("Lora AT timeout retry enter");
    if (Lora_AT_Status != LORA_EXIT_ATCMD)
    {
        SendLora("AT+ENTM\r\n", 9);
        ATCMD_RestartBegin();
    }
}

bool IsExitAtErr(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        return FALSE;
    }
    if (strstr((char const *)strupr(data, len), "ERR") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

void HandLoraATModel()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t LoraAtCmdBuff[50];

    uint8_t *data = GetLoraDataFromBuff(&desiredLen, TRUE);
    uint8_t *LoraAtCmd = NULL;
    if (IsLoraStart(data, desiredLen) && (Lora_AT_Status == LORA_RESTART))
    {
        Lora_AT_Status = LORA_RESTART_OK;
        SendLora("+++", 3);
        //HandleLoraData();
        //DelaySendTask(ReqConfPeriod, ENTER_ATMODLE_TIMEOUT);
        Debug("SendLora(+++)");
    }
    else if (IsEnterATCmdResp(data, desiredLen) && (Lora_AT_Status == LORA_RESTART_OK))
    {
        Lora_AT_Status = LORA_READY_ENTER;
        //DelayUs(1000000);
        SendLora("a", 1);
        //HandleLoraData();
        Debug("SendLora(a)");
    }
    else if (IsEnterATCmdOK(data, desiredLen) && (Lora_AT_Status == LORA_READY_ENTER)) //已经进入AT模式
    {
        CurrLoraCmd = SET_LORA_UART_CMD; //第一条命令
        Lora_AT_Status = LORA_IN_ATCMD;
        //HandleLoraData();
        //snprintf((char *)LoraAtCmdBuff, 50, LORACMD[CurrLoraCmd], Conf.LoraMode ? "FP" : "TRANS");
        Lora_AT_Status = LORA_SENDING_ATCMD;
        SendLora((uint8_t *)LORACMD[CurrLoraCmd], strlen((char const *)LORACMD[CurrLoraCmd]));
        Debug("LORA_IN_ATCMD And Send AT:%s", LORACMD[CurrLoraCmd]);
        CurrLoraCmd++;
    }
    else if (Lora_AT_Status == LORA_SENDING_ATCMD && IsLoraATCmdOK(data, desiredLen))
    {
        Lora_AT_Status = LORA_ATCMD_RESP_OK;
        switch (CurrLoraCmd)
        {
        case SET_LORA_WMODE_CMD:
            snprintf((char *)LoraAtCmdBuff, 50, (char const *)LORACMD[CurrLoraCmd], Conf.LoraMode ? "FP" : "TRANS");
            LoraAtCmd = LoraAtCmdBuff;
            break;
        case SET_LORA_ADDR_CMD:
            snprintf((char *)LoraAtCmdBuff, 50, (char const *)LORACMD[CurrLoraCmd], Conf.LoraAddr);
            LoraAtCmd = LoraAtCmdBuff;
            break;
        case SET_LORA_CH_CMD:
            snprintf((char *)LoraAtCmdBuff, 50, (char const *)LORACMD[CurrLoraCmd], Conf.LoraChan);
            LoraAtCmd = LoraAtCmdBuff;
            break;
        case SET_LORA_SPD_CMD:
            snprintf((char *)LoraAtCmdBuff, 50, (char const *)LORACMD[CurrLoraCmd], Conf.LoraAirRate);
            LoraAtCmd = LoraAtCmdBuff;
            break;
        default:
            LoraAtCmd = LORACMD[CurrLoraCmd];
        }
        Lora_AT_Status = LORA_SENDING_ATCMD;
        SendLora((uint8_t *)LoraAtCmd, strlen((char *)LoraAtCmd));
        CurrLoraCmd++;
        Debug("Send AT:%s", LoraAtCmd);

        if (CurrLoraCmd >= LORA_AT_END)
        {
            Lora_AT_Status = LORA_EXIT_ATCMD;
            LoraStatus = LORA_TRANSFER;
            if (RESET == GetConfFlag(Conf_FLAG_LORA)) //默认配置
            {
                SetConfFlag(Conf_FLAG_LORA);
                if (Conf.DevType == DevType_LoRa_Node) //Lora默认配置完成后，Lora需要注册获取实际使用的配置
                {
                    PushTask(LoraRegister_Task);
                }
                Debug("Default Config OK, Exit Lora AtCmd OK");
            }
            else //注册后重新配置
            {
                if (Conf.DevType == DevType_LoRa_Node) //更新完成注册获取的数据，然后需要获取lora节点连接设备的信息
                {
                    SetConfFlag(Conf_FLAG_LORA_DEV_Done);
                    PushTask(GetLoraDevParam_Task);
                    Debug("Register Config OK, Exit Lora AtCmd OK");
                }
                else
                {
                    Debug("Config Error, Exit Lora AtCmd OK");
                }
            }
        }
    }
    else
    {
        Debug("LoRa Eerror:%s", data);
        if (Lora_AT_Status == LORA_SENDING_ATCMD) //上条命令相应识别，重新发送
        {
            SendLora((uint8_t *)LoraAtCmd, strlen((char *)LoraAtCmd));
            Debug("Retry Send AT:%s", LoraAtCmd);
        }
    }
}

//GPRS AT 模式

#define GPRSCMDNUM (sizeof(GPRSCMD)) / sizeof(GPRSCMD[0])
char *GPSRCMD[] = {
    "AT+UART=115200,\"NONE\",8,1,\"NONE\"\r",
    "AT+SOCKA=\"TCP\",\"%u.%u.%u.%u\",%u\r",
    "AT+SOCKB=\"TCP\",\"%u.%u.%u.%u\",%u\r",
    "AT+SOCKAEN=\"%s\"\r",
    "AT+SOCKBEN=\"%s\"\r",
    "AT+WKMOD=\"NET\"\r",
    "AT+HEARTEN=\"off\"\r",
    "AT+SOCKRSTIM=%u\r",
    "AT+S\r",
    "AT+CFGTF\r",
    "AT+ENTM\r",
    "", //结束配置命令，之后为特殊命令
    "\r\nOK\r\n",
};

void ReadyEnterGPRS_AT()
{
    GPRS_AT_Status = GPRS_READY_ENTER;
    DelaySendTask(200, ENTER_GPRS_AT);
    DevStatus = ENTER_GPRS_AT_CMD;
    DelaySendTask(ReqConfPeriod, ENTER_GPRS_AT_TIMEOUT);
}

void EnterGPRS_AT()
{
    GPRS_AT_Status = GPRS_ENTERING;
    ClearDevBuff(0);
    Debug("Start GPRS +++");
    SendDevice("+++", 3);
}

void EnterGPRS_AT_TIMEOUT()
{
    Debug("EnterGPRS_AT_TIMEOUT");
    if (GPRS_AT_Status != GPRS_EXIT_ATCMD)
    {
        SendDevice((uint8_t *)GPSRCMD[EXIT_GPRS_AT_MODEL], strlen(GPSRCMD[EXIT_GPRS_AT_MODEL]));
        ReadyEnterGPRS_AT();
    }
}

bool isRespOK(uint8_t *data, uint8_t len)
{
    if (len < strlen(GPSRCMD[GPRS_AT_RESP_OK]))
    {
        return FALSE;
    }
    if (strstr((char const *)strupr(data, len), "\r\nOK\r\n") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

void HandGPRSATModel()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetDevDataFromBuff(&desiredLen, TRUE);
    uint8_t *GPRSAtCmd = NULL;
    uint8_t GPRSAtCmdBuff[50];
    if (IsEnterATCmdResp(data, desiredLen) && (GPRS_AT_Status == GPRS_ENTERING)) //准备进入GPRS AT并有GPRS模块的相应
    {
        GPRS_AT_Status = GPRS_RESP;
        //DelayUs(ReqConfPeriod);
        SendDevice("a", 1);

        //是网关则修改Lora为定点模式
        isGPRS_GW = TRUE;
        Conf.LoraMode = 1;
        Conf.SendSlotTime = 2;
        EEPROM_Write_8(loraModeAddr, 1);
        EEPROM_Write_8(SendSlotTimeAddr, 2);
        SetDevType(DevType_GW);

        PushTask(CONF_LORA_PARM); //设备类型确认为GPRS网关，启动Lora参数配置
        Debug("GPRS SendDevice(a)");
    }
    else if (IsEnterATCmdOK(data, desiredLen) && (GPRS_AT_Status == GPRS_RESP)) //确认进入GPRS AT模式，并发送第一条要配置的命令
    {
        GPRS_Curr_Cmd = SET_GPRS_UART_CMD;
        GPRS_AT_Status = GPRS_IN_ATCMD;
        SendDevice((uint8_t *)GPSRCMD[GPRS_Curr_Cmd], strlen(GPSRCMD[GPRS_Curr_Cmd]));
        Debug("GPRS_IN_ATCMD AND send %s", GPSRCMD[GPRS_Curr_Cmd]);
        GPRS_Curr_Cmd++;
        GPRS_AT_Status = GPRS_SENDING_ATCMD;
    }
    else if (isRespOK(data, desiredLen) && GPRS_AT_Status == GPRS_SENDING_ATCMD) //上条命令相应成功
    {
        GPRS_AT_Status = GPRS_ATCMD_RESP_OK;
        if (GPRS_Curr_Cmd >= GPRS_CMD_END) //如果发送完成所有的配置命令，则配置完成
        {
            GPRS_AT_Status = GPRS_EXIT_ATCMD;
            DevStatus = DEV_DATA_TRANSFER;
            SetConfFlag(Conf_FLAG_GPRS);

            Debug("Config OK,Exit GPRS AT Model");
        }
        else //没有配置完成则逐条继续向下配置命令
        {
            switch (GPRS_Curr_Cmd)
            {
            case SET_GPRS_LINKA:
                snprintf((char *)GPRSAtCmdBuff, 50, (char const *)GPSRCMD[GPRS_Curr_Cmd],
                         (*((uint8_t *)&Conf.GPRSLinkAIP + 0)), (*((uint8_t *)&Conf.GPRSLinkAIP + 1)), (*((uint8_t *)&Conf.GPRSLinkAIP + 2)), (*((uint8_t *)&Conf.GPRSLinkAIP + 3)), Conf.GPRSLinkAPort);
                GPRSAtCmd = GPRSAtCmdBuff;
                break;
            case SET_GPRS_LINKB:
                snprintf((char *)GPRSAtCmdBuff, 50, (char const *)GPSRCMD[GPRS_Curr_Cmd],
                         (*((uint8_t *)&Conf.GPRSLinkBIP + 0)), (*((uint8_t *)&Conf.GPRSLinkBIP + 1)), (*((uint8_t *)&Conf.GPRSLinkBIP + 2)), (*((uint8_t *)&Conf.GPRSLinkBIP + 3)), Conf.GPRSLinkBPort);
                GPRSAtCmd = GPRSAtCmdBuff;
                break;
            case SET_GPRS_ENABLE_LINKA:
                snprintf((char *)GPRSAtCmdBuff, 50, (char const *)GPSRCMD[GPRS_Curr_Cmd], Conf.GPRSLinkAEnable ? "on" : "off");
                GPRSAtCmd = GPRSAtCmdBuff;
                break;
            case SET_GPRS_ENABLE_LINKB:
                snprintf((char *)GPRSAtCmdBuff, 50, (char const *)GPSRCMD[GPRS_Curr_Cmd], Conf.GPRSLinkBEnable ? "on" : "off");
                GPRSAtCmd = GPRSAtCmdBuff;
                break;
            case SET_SOCKRSTIM:
                snprintf((char *)GPRSAtCmdBuff, 50, (char const *)GPSRCMD[GPRS_Curr_Cmd], Conf.GPSRSockRestTime);
                GPRSAtCmd = GPRSAtCmdBuff;
                break;
            default:
                GPRSAtCmd = (uint8_t *)GPSRCMD[GPRS_Curr_Cmd];
            }
            SendDevice((uint8_t *)GPRSAtCmd, strlen((char *)GPRSAtCmd));
            Debug("SendDevice:%s", GPRSAtCmd);
            GPRS_Curr_Cmd++;
            GPRS_AT_Status = GPRS_SENDING_ATCMD;
        }
    }
    else //上条命令没有正确相应或者已经退出AT模式，
    {
        Debug("GPRS Error:%s", data);
        if (GPRS_AT_Status == GPRS_SENDING_ATCMD) //如果还在发送命令状态则重试上条命令发送
        {
            SendDevice((uint8_t *)GPSRCMD[GPRS_Curr_Cmd - 1], strlen(GPSRCMD[GPRS_Curr_Cmd - 1]));
            Debug("Retry SendDevice:%s", GPSRCMD[GPRS_Curr_Cmd - 1]);
        }
    }
}

//终止进入GPRS At命令流程
void AbortGPRSAtCmd()
{
    GPRS_AT_Status = GPRS_EXIT_ATCMD;
    DevStatus = DEV_DATA_TRANSFER;
}
//将字符串转为大写
uint8_t *
strupr(uint8_t *str, uint8_t len)
{
    uint8_t i = 0;
    uint8_t *strStart = str;
    while (*str)
    {
        if (*str >= 'a' && *str < 'z')
        {
            *str -= 0x20;
        }
        i++;
        str++;
        if (i >= len)
        {
            break;
        }
    }
    return strStart;
}
//将字符串全转为小写
uint8_t *strlwr(uint8_t *str, uint8_t len)
{
    uint8_t i = 0;
    uint8_t *strStart = str;
    while (*str)
    {
        if (*str >= 'A' && *str < 'Z')
        {
            *str += 0x20;
        }
        i++;
        str++;
        if (i >= len)
        {
            break;
        }
    }
    return strStart;
}