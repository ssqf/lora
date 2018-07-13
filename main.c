#include "lora.h"

int main(void)
{
  InitDevice();
  while (1)
  {
    HandleTask();
  }
  return 0;
}
