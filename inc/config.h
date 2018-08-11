#ifndef __CONFIG_H
#define __CONFIG_H
#include "lora.h"
#include <stdlib.h>

#define CMD_NUMBER 3
#define FITTER_NUMBER 3

//EEPROM 256字节
#define EEPROM_START FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS
#define isInitAddr (EEPROM_START)         //0
#define flagAddr (EEPROM_START)           //4
#define devTypeAddr (flagAddr + 4)        //1
#define connDevTypeAddr (devTypeAddr + 1) //1

#define loraAddr (connDevTypeAddr + 1)     //2
#define loraChanAddr (loraAddr + 2)        //1
#define loraModeAddr (loraChanAddr + 1)    //1
#define loraAirRateAddr (loraModeAddr + 1) //1

#define DevBaudRateAddr (loraAirRateAddr + 1) //1
#define DevDataBitAddr (DevBaudRateAddr + 1)  //1
#define DevStopBitAddr (DevDataBitAddr + 1)   //1
#define DevParityAddr (DevStopBitAddr + 1)    //1

#define LoraBaudRateAddr (DevParityAddr + 1)   //1
#define LoraDataBitAddr (LoraBaudRateAddr + 1) //1
#define LoraStopBitAddr (LoraDataBitAddr + 1)  //1
#define LoraParityAddr (LoraStopBitAddr + 1)   //1

#define SendSlotTimeAddr (LoraParityAddr + 1)    //1
#define HeartBeatTimeAddr (SendSlotTimeAddr + 1) //1

#define CmdLenAddr (HeartBeatTimeAddr + 1)          //1*CMD_NUMBER
#define CmdPeriodAddr (CmdLenAddr + 1 * CMD_NUMBER) //2

#define FitterLenAddr (CmdPeriodAddr + 2 * CMD_NUMBER)      //1
#define FitterTypeAddr (FitterLenAddr + 1 * FITTER_NUMBER)  //1
#define FitterParmAddr (FitterTypeAddr + 1 * FITTER_NUMBER) //2

#define GPRSLinkAIPAddr (Fitter2ParmAddr + 2 * FITTER_NUMBER) //4
#define GPRSLinkAPortAddr (GPRSLinkAIPAddr + 4)               //2
#define GPRSLinkAEnableAddr (GPRSLinkAPortAddr + 2)           //1

#define GPRSLinkBIPAddr (GPRSLinkAEnableAddr + 1)   //4
#define GPRSLinkBPortAddr (GPRSLinkBIPAddr + 4)     //2
#define GPRSLinkBEnableAddr (GPRSLinkBPortAddr + 2) //1

#define CmdFlashAddr (FLASH_PROGRAM_START_PHYSICAL_ADDRESS + ((uint32_t)FLASH_BLOCK_SIZE * (uint32_t)480))
#define FitterFlashAddr (FLASH_PROGRAM_START_PHYSICAL_ADDRESS + ((uint32_t)FLASH_BLOCK_SIZE * (uint32_t)490))
typedef struct
{
  uint32_t SN;
  uint32_t ConfFlag;
  uint8_t DevType;     //网关还是节点,0未知，1GPRS网关，2Lora节点
  uint8_t ConnDevType; //连接的什么设备，0,未知，1电表，2水表，3扬尘，4污水，5地磅

  uint16_t LoraAddr;
  uint8_t LoraChan;
  uint8_t LoraMode;    //0透传，1定点
  uint8_t LoraAirRate; //lora空传速率 默认8

  uint8_t DevBaudRate;  //0 ->600,1->1200,2->4800,3->9600,4->19200,5->38400,6->57600,7->115200
  uint8_t DevDataBit;   //0x0->8,0x10->9
  uint8_t DevStopBit;   //0x00->1,0x20->2,0x30->1.5
  uint8_t DevParityBit; //0x00none,0x04even,0x06odd

  uint8_t LoraBaudRate;
  uint8_t LoraDataBit;
  uint8_t LoraStopBit;
  uint8_t LoraParityBit;

  uint8_t SendSlotTime;  //0(0-300ms)周期数据发送,1(350-650ms)告警类似数据发送,2(700-950ms)网关给节点发送数据
  uint8_t HeartBeatTime; //心跳时间，固定5分钟，在5分钟内的那一秒发送心跳

  uint8_t cmdLen[CMD_NUMBER];     //命令实际长度
  uint16_t cmdPeriod[CMD_NUMBER]; //发送周期，以秒为单位，除时间戳得到发送点

  //fitter中的0xFF为忽略字段
  uint8_t fitterLen[FITTER_NUMBER]; //fitter的长度
  //fiterr的类型，
  //0，匹配直接丢弃
  //1，连续相同fitterParm次相同数据，认为是一次有效数据
  //2，连续相同数据fitterParm秒相同数据，认为一次有效数据
  //3，
  uint8_t fitterType[FITTER_NUMBER];
  //fitter的参数
  uint16_t fitterParm[FITTER_NUMBER];

  uint32_t GPRSLinkAIP;
  uint16_t GPRSLinkAPort;
  uint8_t GPRSLinkAEnable; //0关闭，1，启用
  uint32_t GPRSLinkBIP;
  uint16_t GPRSLinkBPort;
  uint8_t GPRSLinkBEnable; //0关闭，1，启用
  uint16_t GPSRSockRestTime;
} DevConfig;

typedef enum
{
  Conf_FLAG_LORA = (uint32_t)0x01,           //Lora默认配置完成
  Conf_FLAG_LORA_REGIST = (uint32_t)0x02,    //Lora注册完成，配置了获取的数据
  Conf_FLAG_LORA_DEV_Done = (uint32_t)0x04,  //将注册获取的配置更新到Lora中
  Conf_FLAG_GPRS = (uint32_t)0x0100,         //GPRS配置完成
  Conf_FLAG_LORA_DEV = (uint32_t)0x0200,     //lora节点的Dev串口配置完成
  Conf_FLAG_INTEGRITY = (uint32_t)0x80000000 //数据是否完整

} Conf_Flag_TypeDef;

typedef enum
{
  DevType_UnKnown = 0,
  DevType_GW = 1,
  DevType_LoRa_Node = 2
} DeviceType;

typedef enum
{
  LORA_TRANSFER,
  LORA_REGISTER,      //Lora注册
  GET_LORA_DEV_PARAM, //获取Lora Dev口的
  ENTER_LORA_AT_CMD,
} LoRa_Transf_Status;

typedef enum
{
  DEV_DATA_TRANSFER,
  GW_REGISTER,
  ENTER_GPRS_AT_CMD
} Dev_Transf_Status;

extern DevConfig Conf;
extern bool ConfigOver;
extern void InitConf();
extern uint16_t EEPROM_Read_16(uint32_t addr);
extern uint32_t EEPROM_Read_32(uint32_t addr);
extern void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len);
extern void EEPROM_Write_8(uint32_t Address, uint8_t Data);
extern void EEPROM_Write_16(uint32_t Address, uint16_t Data);
extern void EEPROM_Write_32(uint32_t Address, uint32_t Data);
extern void EEPROM_Write_BUFF(uint32_t Address, uint8_t *Data, uint8_t len);
extern void FLASH_Write_Buff(uint32_t Address, uint8_t *Data, uint8_t len);
extern FlagStatus GetConfFlag(Conf_Flag_TypeDef flag);
extern uint32_t readSnFromFlash();
extern void ConfRestoredefault();
extern void SetDevType(DeviceType t);
extern const uint32_t buad[];
extern LoRa_Transf_Status LoraStatus;
extern Dev_Transf_Status DevStatus;
#define EEPROM_Read_8 FLASH_ReadByte
extern void ReStartMCU();
extern void SetConfFlag(Conf_Flag_TypeDef flag);
extern void ClearConfFlag(Conf_Flag_TypeDef flag);
extern void InitResetKey();
extern bool GetResetKeyState();
extern void ResetKeyHandler();
#endif

/* lora的模式说明
网关使用定向模式，地址为FFFF，注意：即使节点地址是FFFF，网关给对其发数据也要带上
节点使用透传模式，直接发送网关即可收到
*/