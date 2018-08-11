#include "lora.h"

int main(void)
{
  InitDevice();
  Debug("Hello LoRa!! Serial Number:%#lX,DevType:%u", Conf.SN, Conf.DevType);
  while (1)
  {
    HandleTask();
  }
}
