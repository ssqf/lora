#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "lora.h"
extern bool PushDataLoraBuff(uint8_t *data, uint8_t len);
extern bool PushDataDevBuff(uint8_t *data, uint8_t len);
extern void HandleDevData();
extern void HandSendLoarData();
extern void HandleLoraData();
extern void ClearLoraBuff(uint8_t len);
extern void ClearDevBuff(uint8_t len);
extern uint8_t *GetLoraDataFromBuff(uint8_t *desiredLen, bool isDel);
extern uint8_t *GetDevDataFromBuff(uint8_t *desiredLen, bool isDel);
extern uint16_t ToUint16(uint8_t *data);
extern uint32_t ToUint32(uint8_t *data);
#endif // !__PROTOCOL_H#define