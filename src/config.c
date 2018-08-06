#include "lora.h"

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
DevConfig conf = {0};
uint32_t readSnFromFlash();
void LoadConfig();

void InitConf()
{
    FLASH_DeInit();
    //FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);
    //FLASH_PowerWaitModeConfig(FLASH_Power_On);
    //FLASH_PowerRunModeConfig(FLASH_Power_On);
    LoadConfig();
    conf.SN = readSnFromFlash();
    //EEPROM_Write_16(Cmd2PeriodAddr, 0x0A0B);
    //EEPROM_Write_8(Cmd2PeriodAddr, 0x0A);
    //EEPROM_Write_8(Cmd2PeriodAddr + 1, 0x0B);
    //conf.cmd2Period = (uint16_t)(FLASH_ReadByte(Cmd2PeriodAddr)) << 8 + (uint16_t)FLASH_ReadByte(Cmd2PeriodAddr);
    //conf.DevType = 1; //0、网关；1、节点
    //conf.LoraAddr = 0x0101;
    //conf.LoraChan = 72;
    //conf.LoraMode = 0; //0、透传；1、定点

    // conf.DevBaudRate = 115200;
    // conf.DevdataBit = 8;
    // conf.DevstopBit = 1;
    // conf.DevParityBit = 0;

    // conf.LoraBaudRate = 115200;
    // conf.LoradataBit = 8;
    // conf.LorastopBit = 1;
    // conf.LoraParityBit = 0;
}

void SaveConf()
{
}

void LoadConfig()
{
    memcpy((uint8_t *)&conf + sizeof(conf.SN), (void const *)EEPROM_START, sizeof(DevConfig) - sizeof(conf.SN));
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
    //FLASH_EraseByte(Address);
    FLASH_ProgramByte(Address, Data);
    //* (PointerAttr uint8_t *)(MemoryAddressCast)Address = Data;

    // while (FLASH_GetFlagStatus(FLASH_FLAG_HVOFF) == RESET)
    // {
    // }
    FLASH_Status_TypeDef status = FLASH_WaitForLastOperation(FLASH_MemType_Data);
    Debug("EEPROM_Write_8 status:%#x", status);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_16(uint32_t Address, uint16_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }
    /* Write one byte - from lowest address*/
    //*((PointerAttr uint8_t *)(MemoryAddressCast)Address) = *((uint8_t *)(&Data));
    /* Write one byte*/
    //*(((PointerAttr uint8_t *)(MemoryAddressCast)Address) + 1) = *((uint8_t *)(&Data) + 1);
    //FLASH_EraseByte(Address);
    FLASH_ProgramByte(Address, *((uint8_t *)(&Data))); //高位在低地址
    //FLASH_EraseByte(Address + 1);
    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
    // while (FLASH_GetFlagStatus(FLASH_FLAG_HVOFF) == RESET)
    // {
    // }
    FLASH_Status_TypeDef status = FLASH_WaitForLastOperation(FLASH_MemType_Data);
    Debug("EEPROM_Write_16 status:%#X", (unsigned int)status);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_32(uint32_t Address, uint32_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);
    /* Wait until Flash Program area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    //FLASH_ProgramWord(Address, Data);
    FLASH_EraseByte(Address);
    FLASH_ProgramByte(Address, *((uint8_t *)(&Data)));
    FLASH_EraseByte(Address + 1);
    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
    FLASH_EraseByte(Address + 2);
    FLASH_ProgramByte(Address + 2, *((uint8_t *)(&Data) + 2));
    FLASH_EraseByte(Address + 3);
    FLASH_ProgramByte(Address + 3, *((uint8_t *)(&Data) + 3));

    FLASH_Status_TypeDef status = FLASH_WaitForLastOperation(FLASH_MemType_Data);
    Debug("EEPROM_Write_32 status:%#X", status);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len)
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    for (uint8_t i = 0; i < len; i++)
    {
        //*(((PointerAttr uint8_t *)(MemoryAddressCast)Address) + i) = *(Data + i);
        FLASH_ProgramByte(Address + i, *(Data + i));
    }

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
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