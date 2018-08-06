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
    LoadConfig();
    conf.SN = readSnFromFlash();
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

    FLASH_ProgramByte(Address, Data);

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
    FLASH_Lock(FLASH_MemType_Data);
}

void EEPROM_Write_16(uint32_t Address, uint16_t Data)
{
    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
    }

    FLASH_ProgramByte(Address, *((uint8_t *)(&Data))); //高位在低地址

    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
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

    FLASH_ProgramByte(Address, *((uint8_t *)(&Data)));
    FLASH_ProgramByte(Address + 1, *((uint8_t *)(&Data) + 1));
    FLASH_ProgramByte(Address + 2, *((uint8_t *)(&Data) + 2));
    FLASH_ProgramByte(Address + 3, *((uint8_t *)(&Data) + 3));

    FLASH_WaitForLastOperation(FLASH_MemType_Data);
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