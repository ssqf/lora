#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <stm8l10x_conf.h>
const uint32_t snAddr = 0xFFFC;
const uint32_t dataStartAddr = 0x0
const uint8_t isInitAddr = 1; //1
const uint8_t devTypeAddr = 2;//1
const uint8_t loraAddr = 2; //2
const uint8_t loraChanAddr = 4;//1
const uint8_t baudRateAddr = 5; //2
const uint8_t dataBitAddr = 7;//1
const uint8_t stopBitAddr = 8;//1
const uint8_t checkAddr = 9;//1
const uint8_t sendSlotTimeAddr=10//2
const uint8_t cmdNumAddr = 12; //1 有几条命令
const uint8_t fitterNumAddr = 13; //1 有几条过滤
const uint8_t cmdStartAddr = 20; //1 每条命令25个字节
const uint8_t fitterSartAddr = 100; //1 每条fitter30字节

typedef DevConfig struct{
  uint32_t SN;
  uint8_t DevType;
  uint16_t LoraAddr;
  uint8_t LoraChan;
  uint8_t BaudRate;
  uint8_t dataBit;
  uint8_t stopBit;
  uint8_t checkBit;
  uint8_t cmd1Addr;
  uint8_t[25] cmd1; //第一个字节为命令实际长度；2、3字节为命令发送的周期
  uint8_t[25] cmd2;
  uint8_t[25] cmd3;
  uint8_t[30] fitter1; //1字节fitter的实际长度；2、fitter的类型(1丢弃，2、连续几次相同上报，3、连续多长时间相同上报时间单位100ms，4、2和3的参数；567位fitter模板中的位是否忽略)
  uint8_t[30] fitter2;  
}

#endif