#include "lora.h"

int main(void)
{
  InitDevice();
  Debug("Hello LoRa!! Serial Number:%#X%X", (unsigned int)(conf.SN >> 16), (unsigned int)conf.SN);
  // EEPROM_Write_8(Cmd1PeriodAddr, 0x22);
  // if (EEPROM_Read_8(Cmd1PeriodAddr) == 0x22)
  // {
  //   Debug("veritfy 8 write ok");
  // }

  // EEPROM_Write_16(Cmd1PeriodAddr, 0x4455);
  // if (EEPROM_Read_16(Cmd1PeriodAddr) == 0x4455)
  // {
  //   Debug("veritfy 16 write ok");
  // }

  // EEPROM_Write_32(Cmd2PeriodAddr, 0xAABBCCDD);
  // if (EEPROM_Read_32(Cmd2PeriodAddr) == 0xAABBCCDD)
  // {
  //   Debug("veritfy 32 write ok");
  // }

  while (1)
  {
    HandleTask();
  }
}
