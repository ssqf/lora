#include "lora.h"
#include "config.h"

LoRa_Transf_Status LoraStatus = LORA_TRANSFER;
Dev_Transf_Status DevStatus = DEV_DATA_TRANSFER;
const uint32_t SNAddr = 0x17FF0; //SN的存放地址
const uint32_t buad[] = {
    600,     //0
    1200,    //1
    4800,    //2
    9600,    //3
    19200,   //4
    38400,   //5
    57600,   //6
    115200}; //7

const DevConfig DefaultDevConfig = {0, 0x80000000, 0, 0, 0xFFFF, 72, 0, 8, 7, 0, 0, 0, 7, 0, 0, 0, 0, 0, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0x276C05B8, 9527, 1, 0x7BCF597E, 9527, 1, 3600};
DevConfig Conf = {0};

void LoadConfig();

void InitConf()
{
    InitResetKey();
    FLASH_DeInit();
    LoadConfig();
    Conf.SN = readSnFromFlash();
}

void LoadConfig()
{
    memcpy((uint8_t *)&Conf + sizeof(Conf.SN), (void const *)EEPROM_START, sizeof(DevConfig) - sizeof(Conf.SN));
}

//只用在写数据内使用
void SetWriteStartFlag()
{
    Conf.ConfFlag &= ~Conf_FLAG_INTEGRITY; //清除完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);
}
void SetWriteEndFlag()
{
    Conf.ConfFlag |= Conf_FLAG_INTEGRITY; //设置完整性标志
    FLASH_ProgramByte(flagAddr, Conf.ConfFlag >> 24);
}

uint32_t readSnFromFlash()
{
    uint32_t sn = 0;
    uint32_t temp = 0;

    temp = FLASH_ReadByte(SNAddr);
    sn |= temp << 24;

    temp = FLASH_ReadByte(SNAddr + 1);
    sn |= temp << 16;

    temp = FLASH_ReadByte(SNAddr + 2);
    sn |= temp << 8;

    sn |= FLASH_ReadByte(SNAddr + 3);
    return sn;
}

void EEPROM_Write_8(uint32_t Address, uint8_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);
    /* Wait until Flash Program area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    SetWriteStartFlag();
    FLASH_ProgramByte(Address, Data);
    SetWriteEndFlag();

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_16(uint32_t Address, uint16_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }
    SetWriteStartFlag();
    FLASH_ProgramByte(Address, *((uint8_t *)(&Data))); //高位在低地址
    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
    SetWriteEndFlag();
    // while (FLASH_GetFlagStatus(FLASH_FLAG_HVOFF) == RESET)
    // {
    // }
    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_32(uint32_t Address, uint32_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);

    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }
    //FLASH_ProgramWord(Address, Data);
    SetWriteStartFlag();
    FLASH_ProgramByte(Address, *((uint8_t *)(&Data)));
    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
    FLASH_ProgramByte(Address + 2, *((uint8_t *)(&Data) + 2));
    FLASH_ProgramByte(Address + 3, *((uint8_t *)(&Data) + 3));
    SetWriteEndFlag();

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len)
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    SetWriteStartFlag();
    for (uint8_t i = 0; i < len; i++)
    {
        //*(((PointerAttr uint8_t *)(MemoryAddressCast)Address) + i) = *(Data + i);
        FLASH_ProgramByte(Address + i, *(Data + i));
    }
    SetWriteEndFlag();

    //BUG 有可能写数据错误
    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
}

void FLASH_Write_Buff(uint32_t Address, uint8_t *Data, uint8_t len)
{
    FLASH_Unlock(FLASH_MemType_Program);
    while (FLASH_GetFlagStatus(FLASH_FLAG_PUL) == RESET)
    {
    }

    for (uint8_t i = 0; i < len; i++)
    {
        //*(((PointerAttr uint8_t *)(MemoryAddressCast)Address) + i) = *(Data + i);
        FLASH_ProgramByte(Address + i, *(Data + i));
    }

    FLASH_WaitForLastOperation(FLASH_MemType_Program);
    FLASH_Lock(FLASH_MemType_Program);
}

uint16_t EEPROM_Read_16(uint32_t addr)
{
    uint16_t result = 0;
    result = FLASH_ReadByte(addr) << 8;
    result |= FLASH_ReadByte(addr + 1);
    return result;
}

uint32_t EEPROM_Read_32(uint32_t addr)
{
    uint32_t result = 0;

    *((uint8_t *)&result) = FLASH_ReadByte(addr);
    *(((uint8_t *)&result) + 1) = FLASH_ReadByte(addr + 1);
    *(((uint8_t *)&result) + 2) = FLASH_ReadByte(addr + 2);
    *(((uint8_t *)&result) + 3) = FLASH_ReadByte(addr + 3);
    return result;
}

void SetDevType(DeviceType t)
{
    EEPROM_Write_8(devTypeAddr, t);
    Conf.DevType = t;
}

FlagStatus GetConfFlag(Conf_Flag_TypeDef flag)
{
    return (FlagStatus)((Conf.ConfFlag & ((uint32_t)flag)) != 0);
}

void SetConfFlag(Conf_Flag_TypeDef flag)
{
    uint32_t currFlag = Conf.ConfFlag | flag;
    EEPROM_Write_32(flagAddr, currFlag);
    Conf.ConfFlag |= flag;
}

void ClearConfFlag(Conf_Flag_TypeDef flag)
{
    uint32_t currFlag = Conf.ConfFlag & ~flag;
    EEPROM_Write_32(flagAddr, currFlag);
    Conf.ConfFlag &= ~flag;
}

void ConfRestoredefault()
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }
    // uint8_t len = sizeof(DevConfig);
    // for (uint8_t i = 0; i < sizeof(DevConfig) - sizeof(Conf.SN); i++)
    // {
    //     //*(((PointerAttr uint8_t *)(MemoryAddressCast)Address) + i) = *(Data + i);
    //     FLASH_ProgramByte((void const *)EEPROM_START + i, *((uint8_t *)&DefaultDevConfig + sizeof(Conf.SN) + i));
    // }
    memcpy((void *)EEPROM_START, (const void *)((uint8_t *)&DefaultDevConfig + sizeof(Conf.SN)), sizeof(DevConfig) - sizeof(Conf.SN));

    //BUG 有可能写数据错误
    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
    ReStartMCU();
}

void ReStartMCU()
{
    Debug("ReStartMCU");
    WWDG_SWReset();
}

//初始化复位按键
void InitResetKey()
{
    GPIO_Init(GPIOE, GPIO_Pin_5, GPIO_Mode_In_PU_No_IT);
}

//获取是否安下，TRUE 按下
bool GetResetKeyState()
{
    return (bool)(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5));
}

void ResetKeyHandler()
{
    static uint8_t keyDownTick = 0;
    static bool lastState = FALSE;
    bool state = GetResetKeyState();

    if (lastState != state)
    {
        lastState = state;
        if (state == FALSE) //按键松开
        {
            if (keyDownTick > COUNTER1S) //按下1s以上则重启
            {
                ReStartMCU();
            }
            keyDownTick = 0;
        }
        else
        { //按键按下
            keyDownTick++;
        }
    }

    if (state == TRUE && lastState == state) //按键一直按下
    {
        if (keyDownTick < 0xFF)
        {
            keyDownTick++;
        }
        if (keyDownTick > COUNTER5S) //按键超过5秒回复默认配置
        {
            Debug("Restore default Settings");
            ConfRestoredefault();
        }
    }
}
