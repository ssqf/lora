#include "lora.h"

const uint32_t snAddr = 0xFFFC;
const uint32_t dataStartAddr = 0x0;
const uint8_t isInitAddr = 1;        //1
const uint8_t devTypeAddr = 2;       //1
const uint8_t loraAddr = 2;          //2
const uint8_t loraChanAddr = 4;      //1
const uint8_t baudRateAddr = 5;      //2
const uint8_t dataBitAddr = 7;       //1
const uint8_t stopBitAddr = 8;       //1
const uint8_t checkAddr = 9;         //1
const uint8_t sendSlotTimeAddr = 10; //2
const uint8_t cmdNumAddr = 12;       //1 有几条命令
const uint8_t fitterNumAddr = 13;    //1 有几条过滤
const uint8_t cmdStartAddr = 20;     //1 每条命令25个字节
const uint8_t fitterSartAddr = 100;  //1 每条fitter30字节

DevConfig conf;
