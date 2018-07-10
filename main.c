#include "lora.h"

int main(void)
{
  InitDevice();
  //showString("device start\n");
  //int ans;
  while (1)
  {
    // ans = getchar();
    // putchar(ans);
    //SendDevice("Hello World\n", 12);
    //SendDevice("Hello World1\n", 13);
    handleTask();
  }
  return 0;
}
