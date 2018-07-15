#ifndef __AT_CMD_H
#define __AT_CMD_H

typedef enum
{
    LORA_TRANSFER,
    LORA_RESTART,
    LORA_RESTART_OK,
    LORA_READY_ENTER,
    LORA_IN_ATCMD,
    LORA_READY_EXIT_ATCMD,
    LORA_EXIT_ATCMD

} ATCmd_Status;
extern ATCmd_Status AT_Status;

extern void SetHostWakeState(BitAction state);
extern void EnterLoraConfMode();

extern void SetWakeState(BitAction state);
extern void InitLoraPin();

extern void HandLoraATModel();

extern void ATCMD_RestartBegin();
extern void ATCMD_ResartEnd();
extern void EnterAtModelTimeout();

extern bool IsEnterLoraConfig(uint8_t *data, uint8_t len);
extern bool IsLoraStart(uint8_t *data, uint8_t len);
extern bool IsEnterATCmdResp(uint8_t *data, uint8_t len);
extern bool IsEnterATCmdOK(uint8_t *data, uint8_t len);
extern bool IsRedayExitATCmd(uint8_t *data, uint8_t len);
extern bool IsExitATCmdOK(uint8_t *data, uint8_t len);
extern bool IsExitAtErr(uint8_t *data, uint8_t len);
#endif // !__AT_CMD_H#define
