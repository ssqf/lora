#include "lora.h"

int main(void)
{
  InitDevice();
  Debug("Hello LoRa!! Serial Number:%#X%X", (unsigned int)(conf.SN >> 16), (unsigned int)conf.SN);

  while (1)
  {
    HandleTask();
  }
}
