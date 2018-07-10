#ifndef __CONFIG_H
#define __CONFIG_H
#include "lora.h"
#include <stdlib.h>

typedef struct
{
  uint32_t SN;
  uint8_t DevType;
  uint16_t LoraAddr;
  uint8_t LoraChan;
  uint8_t BaudRate;
  uint8_t dataBit;
  uint8_t stopBit;
  uint8_t checkBit;
  uint8_t cmd1Addr;
  uint8_t cmdNum;
  uint8_t fitterNum;
  uint8_t *cmd1; //第一个字节为命令实际长度；2、3字节为命令发送的周期
  uint8_t *cmd2;
  uint8_t *cmd3;
  uint8_t *fitter1; //1字节fitter的实际长度；2、fitter的类型(1丢弃，2、连续几次相同上报，3、连续多长时间相同上报时间单位100ms，4、2和3的参数；567位fitter模板中的位是否忽略)
  uint8_t *fitter2;
} DevConfig;
extern DevConfig conf;

#endif