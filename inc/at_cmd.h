#ifndef __AT_CMD_H
#define __AT_CMD_H

typedef enum
{
    //LORA进入AT命令的状态
    LORA_TRANSFER,
    LORA_RESTART,
    LORA_RESTART_OK,
    LORA_READY_ENTER,
    LORA_ENTERING,
    LORA_RESP,
    LORA_SENDING_ATCMD,
    LORA_ATCMD_RESP_OK,
    LORA_IN_ATCMD,
    LORA_READY_EXIT_ATCMD,
    LORA_EXIT_ATCMD,

} ATCmd_Status;

typedef enum
{
    // GPSR 进入AT命令的状态
    DEV_DATA_TRANSFER,
    GPRS_READY_ENTER,
    GPRS_ENTERING,
    GPRS_RESP,
    GPRS_IN_ATCMD,
    GPRS_SENDING_ATCMD,
    GPRS_ATCMD_RESP_OK,
    GPRS_READY_EXIT_ATCMD,
    GPRS_EXIT_ATCMD
} GPRS_ATCmd_Status;
extern ATCmd_Status AT_Status;
extern GPRS_ATCmd_Status GPRS_AT_Status;

extern void
SetHostWakeState(BitAction state);
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
extern void InitGPRSConf();
extern void InitLoraConf();
extern void EnterGPRS_AT();
extern void EnterGPRS_AT_TIMEOUT();
extern void ReadyEnterGPRS_AT();
extern void HandGPRSATModel();
extern uint8_t *strlwr(uint8_t *str, uint8_t len);
extern uint8_t *strupr(uint8_t *str, uint8_t len);
#endif // !__AT_CMD_H#define
