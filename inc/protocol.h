#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "lora.h"
extern bool PushDataLoraBuff(uint8_t *data, uint8_t len);
extern bool PushDataDevBuff(uint8_t *data, uint8_t len);
extern void HandleDevData();
extern void HandleLoraDate();
extern void HandSendLoarData();
#endif // !__PROTOCOL_H#define