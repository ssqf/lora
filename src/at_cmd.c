#include "lora.h"

ATCmd_Status AT_Status = LORA_TRANSFER;
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

void ATCMD_RestartBegin()
{
    SetResetState(RESET);
    DelaySendTask(10, ATCMD_RESTARTEND); //拉低10ms Lora复位
}

void ATCMD_ResartEnd()
{
    SetResetState(SET);
}

// void SendEnterConfigAtcmd()
// {
//     SendLora("+++", 3);
// }

//进AT命令模式步骤
//1、复位LoRa模组
//2、复位后2秒内给LoRa发送+++
//3、收到LoRa返回的a，再给LoRa发送a
//4、收到LoRa返回的+OK表示进入AT 命令模式
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
    if (strstr((char const *)data, "+OK") != NULL)
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
    if (strstr((char const *)data, "AT+ENTM\r\n") != NULL)
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
    if (strstr((char const *)data, "\r\n\r\nOK\r\n") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

void EnterAtModelTimeout()
{
    if (AT_Status != LORA_IN_ATCMD)
    {
        AT_Status = LORA_TRANSFER;
        ClearDevBuff(0);
    }
}

bool IsExitAtErr(uint8_t *data, uint8_t len)
{
    if (len < 3)
    {
        return FALSE;
    }
    if (strstr((char const *)data, "ERR") != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

void HandLoraATModel()
{
    uint8_t desiredLen;
    desiredLen = 0;
    uint8_t *data = GetLoraDataFromBuff(&desiredLen, FALSE);
    if (IsLoraStart(data, desiredLen) && (AT_Status == LORA_RESTART))
    {
        AT_Status = LORA_RESTART_OK;
        SendLora("+++", 3);
        HandleLoraData();
        Debug("SendLora(+++)");
    }
    else if (IsEnterATCmdResp(data, desiredLen) && (AT_Status == LORA_RESTART_OK))
    {
        AT_Status = LORA_READY_ENTER;
        //DelayUs(1000000);
        SendLora("a", 1);
        HandleLoraData();
        Debug("SendLora(a)");
    }
    else if (IsEnterATCmdOK(data, desiredLen) && (AT_Status == LORA_READY_ENTER))
    {
        AT_Status = LORA_IN_ATCMD;
        HandleLoraData();
        Debug("LORA_IN_ATCMD");
    }
    else if (IsExitATCmdOK(data, desiredLen) && (AT_Status == LORA_READY_EXIT_ATCMD))
    {
        AT_Status = LORA_EXIT_ATCMD;
        AT_Status = LORA_TRANSFER;
        HandleLoraData();
        Debug("ExitAt OK");
    }
    else if (AT_Status == LORA_READY_EXIT_ATCMD && IsExitAtErr(data, desiredLen))
    {
        SendLora("AT+ENTM\r\n", 9);
        Debug("ExitAt Error");
    }
    else
    {
        HandleLoraData(); //特殊处理完再处理发给串口
    }

    //HandleLoraData(); //特殊处理完再处理发给串口
}