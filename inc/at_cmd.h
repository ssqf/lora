#ifndef __AT_CMD_H
#define __AT_CMD_H

typedef enum
{
    //LORA进入AT命令的状态
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
    GPRS_READY_ENTER,
    GPRS_ENTERING,
    GPRS_RESP,
    GPRS_IN_ATCMD,
    GPRS_SENDING_ATCMD,
    GPRS_ATCMD_RESP_OK,
    GPRS_READY_EXIT_ATCMD,
    GPRS_EXIT_ATCMD
} GPRS_ATCmd_Status;

typedef enum
{
    SET_LORA_UART_CMD = 0,
    SET_LORA_WMODE_CMD,
    SET_LORA_ADDR_CMD,
    SET_LORA_CH_CMD,
    SET_LORA_SPD_CMD,
    SAVE_LORA_CONF_CMD,
    EXIT_LORA_AT_CMD,
    LORA_AT_END,
    LORA_RESP_OK_CMD,

} LORA_CMD_NAME;

typedef enum
{
    SET_GPRS_UART_CMD = 0,    //"AT+UART=115200,NONE,8,1,NONE\r"
    SET_GPRS_LINKA,           //"AT+SOCKA=TCP,39.108.5.184,9527\r"
    SET_GPRS_LINKB,           //"AT+SOCKB=TCP,123.207.89.126,9527\r"
    SET_GPRS_ENABLE_LINKA,    //"AT+SOCKAEN=ON\r"
    SET_GPRS_ENABLE_LINKB,    //"AT+SOCKBEN=ON\r"
    SET_GPRS_NETMODEL,        //"AT+WKMOD=NET\r"
    SET_HB_OFF,               //AT+HEARTEN=\"off\"\r",
    SET_SOCKRSTIM,            //"AT+SOCKRSTIM=3600\r",
    SAVE_GPRS_CONFIG,         //"AT+S\r"
    SAVE_GPRS_CONFIG_DEFUALT, //"AT+CFGTF\r"
    EXIT_GPRS_AT_MODEL,       //"AT+ENTM\r"
    GPRS_CMD_END,             //GPRS命令结束
    GPRS_AT_RESP_OK,          //"\r\nOK\r\n"

} GPRS_CMD_NAME;

extern ATCmd_Status AT_Status;
extern GPRS_ATCmd_Status GPRS_AT_Status;
extern bool isGPRS_GW;
extern uint8_t EnterGPRS_AtTryTimes;

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
extern void AbortGPRSAtCmd();
extern uint8_t *strlwr(uint8_t *str, uint8_t len);
extern uint8_t *strupr(uint8_t *str, uint8_t len);
#endif // !__AT_CMD_H#define
