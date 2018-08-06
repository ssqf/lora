#include "lora.h"

ATCmd_Status AT_Status = LORA_TRANSFER;
GPRS_ATCmd_Status GPRS_AT_Status = DEV_DATA_TRANSFER;

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

#define LORACMDNUM 7
char *LORACMD[LORACMDNUM] = {
    "AT+UART=115200,8,1,NONE,NFC\r\n",
    "AT+WMODE=%s\r\n",
    "AT+ADDR=%d\r\n",
    "AT+CH=%d",
    "AT+CFGTF\r\n",
    "AT+ENTM\r\n",
    "\r\nOK\r\n",
};
uint8_t CurrLoraCmdIndex = 0;

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
    AT_Status = LORA_RESTART;
    CurrLoraCmdIndex = 0;
}

void ATCMD_ResartEnd()
{
    SetResetState(SET);
}

bool IsEnterLoraConfig(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        return FALSE;
    }
    if (strstr((char const *)data, "+++") != NULL)
    {
        return TRUE;
    }
    return FALSE;
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

bool IsRedayExitATCmd(uint8_t *data, uint8_t len)
{
    if (len < 9)
    {
        return FALSE;
    }
    if (strstr((char const *)strupr(data, len), "AT+ENTM\r\n") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

bool IsExitATCmdOK(uint8_t *data, uint8_t len)
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
    if (AT_Status != LORA_TRANSFER)
    {
        SendLora("AT+ENTM\r\n", 9);
        ATCMD_RestartBegin();
        Debug("Lora AT timeout rep enter");
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

uint8_t LoraAtCmdBuff[50];

void HandLoraATModel()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, TRUE);
    if (IsLoraStart(data, desiredLen) && (AT_Status == LORA_RESTART))
    {
        AT_Status = LORA_RESTART_OK;
        SendLora("+++", 3);
        //HandleLoraData();
        DelaySendTask(10000, ENTER_ATMODLE_TIMEOUT);
        Debug("SendLora(+++)");
    }
    else if (IsEnterATCmdResp(data, desiredLen) && (AT_Status == LORA_RESTART_OK))
    {
        AT_Status = LORA_READY_ENTER;
        //DelayUs(1000000);
        SendLora("a", 1);
        //HandleLoraData();
        Debug("SendLora(a)");
    }
    else if (IsEnterATCmdOK(data, desiredLen) && (AT_Status == LORA_READY_ENTER))
    {
        AT_Status = LORA_IN_ATCMD;
        //HandleLoraData();
        //snprintf((char *)LoraAtCmdBuff, 50, LORACMD[CurrLoraCmdIndex], conf.LoraMode ? "FP" : "TRANS");
        AT_Status = LORA_SENDING_ATCMD;
        SendLora((uint8_t*)LORACMD[CurrLoraCmdIndex], strlen((char const *)LORACMD[CurrLoraCmdIndex]));
        Debug("LORA_IN_ATCMD And Send AT:%s", LORACMD[CurrLoraCmdIndex]);
        CurrLoraCmdIndex++;
    }
    else if (AT_Status == LORA_SENDING_ATCMD && IsLoraATCmdOK(data, desiredLen))
    {
        AT_Status = LORA_ATCMD_RESP_OK;
        if (CurrLoraCmdIndex == 1)
        {
            snprintf((char *)LoraAtCmdBuff, 50, LORACMD[CurrLoraCmdIndex], conf.LoraMode ? "FP" : "TRANS");
            AT_Status = LORA_SENDING_ATCMD;
            SendLora((uint8_t *)LoraAtCmdBuff, strlen((char *)LoraAtCmdBuff));
            CurrLoraCmdIndex++;
            Debug("Send AT:%s", LoraAtCmdBuff);
        }
        else if (CurrLoraCmdIndex == 2)
        {

            snprintf((char *)LoraAtCmdBuff, 50, LORACMD[CurrLoraCmdIndex], conf.LoraAddr);
            AT_Status = LORA_SENDING_ATCMD;
            CurrLoraCmdIndex++;
            SendLora((uint8_t *)LoraAtCmdBuff, strlen((char *)LoraAtCmdBuff));
            Debug("Send AT:%s", LoraAtCmdBuff);
        }
        else if (CurrLoraCmdIndex == 2)
        {

            snprintf((char *)LoraAtCmdBuff, 50, LORACMD[CurrLoraCmdIndex], conf.LoraChan);
            AT_Status = LORA_SENDING_ATCMD;
            CurrLoraCmdIndex++;
            SendLora((uint8_t *)LoraAtCmdBuff, strlen((char *)LoraAtCmdBuff));
            Debug("Send AT:%s", LoraAtCmdBuff);
        }
        else
        {
            SendLora((uint8_t *)LORACMD[CurrLoraCmdIndex], strlen(LORACMD[CurrLoraCmdIndex]));
            Debug("Send AT:%s", LORACMD[CurrLoraCmdIndex]);
            CurrLoraCmdIndex++;
            AT_Status = LORA_SENDING_ATCMD;
            if (CurrLoraCmdIndex >= LORACMDNUM)
            {
                AT_Status = LORA_EXIT_ATCMD;
                AT_Status = LORA_TRANSFER;
                Debug("Config OK, Exit Lora AtCmd OK");
            }
        }
    }
    else
    {
        Debug("LoRa Eerror:%s", data);
    }
}

//GPRS AT 模式
#define GPRSCMDNUM 11
char *GPSRCMD[GPRSCMDNUM] = {
    "AT+UART=115200,\"NONE\",8,1,\"NONE\"\r",
    "AT+SOCKA=\"TCP\",\"39.108.5.184\",22222\r",
    "AT+SOCKB=\"TCP\",\"123.207.89.126\",9527\r",
    "AT+SOCKAEN=\"on\"\r",
    "AT+SOCKBEN=\"on\"\r",
    "AT+WKMOD=\"NET\"\r",
    "AT+HEARTEN=\"off\"\r",
    "AT+CFGTF\r",
    "AT+S\r",
    "AT+ENTM\r",
    "\r\nOK\r\n",
};

uint8_t GPRS_CURR_CMD = 0;
#define SET_GPRS_UART_CMD 0        //"AT+UART=115200,NONE,8,1,NONE\r"
#define SET_GPRS_LINKA 1           //"AT+SOCKA=TCP,39.108.5.184,9527\r"
#define SET_GPRS_LINKB 2           //"AT+SOCKB=TCP,123.207.89.126,9527\r"
#define SET_GPRS_ENABLE_LINKA 3    //"AT+SOCKAEN=ON\r"
#define SET_GPRS_ENABLE_LINKB 4    //"AT+SOCKBEN=ON\r"
#define SET_GPRS_NETMODEL 5        //"AT+WKMOD=NET\r"
#define SAVE_GPRS_CONFIG 6         //"AT+S\r"
#define SAVE_GPRS_CONFIG_DEFUALT 7 //"AT+CFGTF\r"
#define EXIT_GPRS_AT_MODEL 8       //"AT+ENTM\r"
#define GPRS_AT_RESP_OK 10         //"\r\nOK\r\n"

void ReadyEnterGPRS_AT()
{
    GPRS_AT_Status = GPRS_READY_ENTER;
    GPRS_CURR_CMD = 0;
    DelaySendTask(200, ENTER_GPRS_AT);
}

void EnterGPRS_AT()
{
    GPRS_AT_Status = GPRS_ENTERING;
    ClearDevBuff(0);
    Debug("Start GPRS +++");
    SendDevice("+++", 3);
    DelaySendTask(10000, ENTER_GPRS_AT_TIMEOUT);
}

void EnterGPRS_AT_TIMEOUT()
{
    if (GPRS_AT_Status != DEV_DATA_TRANSFER)
    {
        SendDevice((uint8_t*)GPSRCMD[GPRSCMDNUM - 2], strlen(GPSRCMD[GPRSCMDNUM - 2]));
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
    if (IsEnterATCmdResp(data, desiredLen) && (GPRS_AT_Status == GPRS_ENTERING))
    {
        GPRS_AT_Status = GPRS_RESP;
        //DelayUs(10000);
        SendDevice("a", 1);
        Debug("SendDevice(a)");
    }
    else if (IsEnterATCmdOK(data, desiredLen) && (GPRS_AT_Status == GPRS_RESP))
    {
        GPRS_AT_Status = GPRS_IN_ATCMD;
        SendDevice((uint8_t *)GPSRCMD[GPRS_CURR_CMD], strlen(GPSRCMD[GPRS_CURR_CMD]));
        Debug("GPRS_IN_ATCMD AND send %s", GPSRCMD[GPRS_CURR_CMD]);
        GPRS_CURR_CMD++;
        GPRS_AT_Status = GPRS_SENDING_ATCMD;
    }
    else if (isRespOK(data, desiredLen) && GPRS_AT_Status == GPRS_SENDING_ATCMD)
    {
        GPRS_AT_Status = GPRS_ATCMD_RESP_OK;
        if (GPRS_CURR_CMD >= GPRSCMDNUM - 1)
        {
            //GPRS_AT_Status = GPRS_EXIT_ATCMD;
            GPRS_AT_Status = DEV_DATA_TRANSFER;
            Debug("Config OK,Exit GPRS AT Model");
        }
        else
        {
            SendDevice((uint8_t *)GPSRCMD[GPRS_CURR_CMD], strlen(GPSRCMD[GPRS_CURR_CMD]));
            Debug("SendDevice:%s", GPSRCMD[GPRS_CURR_CMD]);
            GPRS_CURR_CMD++;
            GPRS_AT_Status = GPRS_SENDING_ATCMD;
        }
    }
    else
    {
        Debug("GPRS Error:%s", data);
    }
}

uint8_t *strupr(uint8_t *str, uint8_t len)
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