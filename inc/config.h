#ifndef __CONFIG_H
#define __CONFIG_H
#include "lora.h"
#include <stdlib.h>

//EEPROM 256字节
//const uint32_t dataStartAddr = 0x0;
#define EEPROM_START FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS
#define isInitAddr (EEPROM_START) //0
#define flagAddr (EEPROM_START) //4
#define devTypeAddr (flagAddr + 4) //1
#define connDevTypeAddr (devTypeAddr + 1)//1

#define loraAddr (connDevTypeAddr + 1) //2
#define loraChanAddr (loraAddr + 2)    //1
#define loraMode (loraChanAddr + 1)    //1
#define loraAirRate (loraMode + 1)     //1

#define DevBaudRateAddr (loraAirRate + 1)    //1
#define DevDataBitAddr (DevBaudRateAddr + 1) //1
#define DevStopBitAddr (DevDataBitAddr + 1)  //1
#define DevParityAddr (DevStopBitAddr + 1)   //1

#define LoraBaudRateAddr (DevParityAddr + 1)   //1
#define LoraDataBitAddr (LoraBaudRateAddr + 1) //1
#define LoraStopBitAddr (LoraDataBitAddr + 1)  //1
#define LoraParityAddr (LoraStopBitAddr + 1)   //1

#define SendSlotTimeAddr (LoraParityAddr + 1) //1

#define Cmd1LenAddr (SendSlotTimeAddr + 1)
#define Cmd1PeriodAddr (Cmd1LenAddr + 1)//2
#define Cmd2LenAddr (Cmd1PeriodAddr + 2)
#define Cmd2PeriodAddr (Cmd2LenAddr + 1)//2
#define Cmd3LenAddr (Cmd2PeriodAddr + 2)
#define Cmd3PeriodAddr (Cmd3LenAddr + 1)//2

#define Fitter1LenAddr (Cmd3PeriodAddr + 2)
#define Fitter1TypeAddr (Fitter1LenAddr + 1)
#define Fitter1ParmAddr (Fitter1TypeAddr + 1)//2
#define Fitter2LenAddr (Fitter1ParmAddr + 2)
#define Fitter2TypeAddr (Fitter2LenAddr + 1)
#define Fitter2ParmAddr (Fitter2TypeAddr + 1)//2

typedef struct DeviceConfig
{
  uint32_t SN;
  uint32_t ConfFlag;
  uint8_t DevType;     //网关还是节点,0未知，1网关，2节点
  uint8_t ConnDevType; //连接的什么设备，1电表，2水表，3扬尘，4污水，5地磅

  uint16_t LoraAddr;
  uint8_t LoraChan;
  uint8_t LoraMode;
  uint8_t LoraAirRate; //lora空传速率 默认10=A

  uint8_t DevBaudRate;  //0 ->600,1->1200,2->4800,3->9600,4->19200,5->38400,6->57600,7->115200
  uint8_t DevdataBit;   //0->8,1->9
  uint8_t DevstopBit;   //0x00->1,0x20->2,0x30->1.5
  uint8_t DevParityBit; //0x00none,0x04even,0x06odd

  uint8_t LoraBaudRate;
  uint8_t LoradataBit;
  uint8_t LorastopBit;
  uint8_t LoraParityBit;

  uint8_t SendSlotTime; //0(0-300ms),1(350-650ms),2(700-950ms)

  //第一个字节为命令实际长度；2、3字节为命令发送的周期
  uint8_t cmd1Len;
  uint16_t cmd1Period; //发送周期，以秒为单位
  uint8_t cmd2Len;
  uint16_t cmd2Period;
  uint8_t cmd3Len;
  uint16_t cmd3Period;

  //fitter中的0xFF为忽略字段
  uint8_t fitter1Len;
  uint8_t fitter1Type;
  uint16_t fitter1Parm;
  uint8_t fitter2Len;
  uint8_t fitter2Type;
  uint16_t fitter2Parm;
} DevConfig;

extern DevConfig conf;
extern bool ConfigOver;
extern void InitConf();
extern uint16_t EEPROM_Read_16(uint32_t addr);
extern uint32_t EEPROM_Read_32(uint32_t addr);
extern void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len);
extern void EEPROM_Write_8(uint32_t Address, uint8_t Data);
extern void EEPROM_Write_16(uint32_t Address, uint16_t Data);
extern void EEPROM_Write_32(uint32_t Address, uint32_t Data);
extern void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len);
extern const uint32_t buad[];
#define EEPROM_Read_8 FLASH_ReadByte
#endif