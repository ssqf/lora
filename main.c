#include "lora.h"

int main(void)
{
  InitDevice();
  Debug("Hello LoRa!!");
  while (1)
  {
    HandleTask();
  }
}
