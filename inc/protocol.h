#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "lora.h"

#define FrameMinLen (uint8_t)(sizeof(Frame) - sizeof(uint8_t *)) //帧最小长度，
#define FrameFixedLen (sizeof(Frame) - 4 - sizeof(uint8_t *))    // 帧固定长度 减去帧头帧尾和data的指针

typedef enum
{
    LORA_DATA_TRANSF = 0,   //我就是传个数据
    I_AM_GW = 1,            //我是网关我要注册
    I_AM_LORA_NODE,         //我是Lora节点，请给我分配Lora地址，信道，和工作模式
    NODE_CONN_WHICH_DEVICE, //我是Lora节点，我连的是那个设备，波特率，设备类型是那些
    SYNC_TIME,              //我是Lora节点我要同步的时间
    HEARTBEAT,              //我心在跳
    ACK,                    //确认收到该帧
    DEVCMD,                 //配置定时发送给设备的命令
    DEVFITTER,              //服务器下发fitter
    Who_Ara_You,            //服务器查询是那个网关
} FrameDataType;

typedef enum
{
    FRAME_OK,
    FRAME_ERROR,
    DATALEN_NotENOUGH,
    SUMCHECK_ERROR,
} FramePraseRESULT;

typedef enum
{
    Dev_DianBiao = 0,
    Dev_ShuiBiao,
    Dev_HuanJing,
} NodeDevType;
/*
协议说明
帧头	    帧长	     帧号	  SN             版本	 数据类型	设备类型   信道	      地址		  data	       校验和	    帧尾
2byte	    1byte	    1byte	 4byte	        1byte	1byte	  1byte     1byte	   2byte	   0-n	        1byte	    2byte
0x5a55	    0-0xFF	    0-0xFF	0-0xFFFFFFFF	0-0xFF	0-FF	  0-FF      0-FF	   0-FFFF      0-0xFF	    0-0xFF      0x6A69

帧长不包含帧头帧尾

*/
typedef struct
{
    uint16_t Head;    //5A55
    uint8_t FrameLen; //帧长不包含帧头帧尾,但要包含数据内容的所有数据
    uint8_t FrameNum;
    uint32_t SerailNum;
    uint8_t Verison;
    uint8_t DataType;
    uint8_t ConnDevType;
    uint8_t LoraChanNum;
    uint16_t LoraAddr;
    uint8_t *Data;
    uint8_t SumCheck;
    uint16_t Tail; //6A69
} Frame;

extern bool PushDataLoraBuff(uint8_t *data, uint8_t len);
extern bool PushDataDevBuff(uint8_t *data, uint8_t len);
extern void HandleDevData();
//extern void HandSendLoarData();
extern void HandleLoraData();
extern void ClearLoraBuff(uint8_t len);
extern void ClearDevBuff(uint8_t len);
extern void GwRegister();
extern void LoraRegister();
extern void GetLoraDevConf();
extern void LoraRespAck(Frame f);
extern uint8_t *GetLoraDataFromBuff(uint8_t *desiredLen, bool isDel);
extern uint8_t *GetDevDataFromBuff(uint8_t *desiredLen, bool isDel);
extern uint16_t ToUint16(uint8_t *data);
extern uint32_t ToUint32(uint8_t *data);
extern void SendDataToLoraNode(Frame f);
extern void HandleServData();
extern Frame PraseToFrame(uint8_t *data, uint8_t *len, FramePraseRESULT *result);
extern uint8_t GetFrameDataLen(Frame f);
extern void LoraRegisterDone(Frame f);
extern void GetLoraDevConfDone(Frame f);
extern void DevDataByFrameSendLoar();
extern void GwRespServ();
extern Frame CreateFrame(FrameDataType dataType, uint8_t *data, uint8_t len);
extern void HandleLoraNodeData();
extern void LoraDataTransfToDev();
extern void DevDataByFrameSendLoar();
extern uint8_t GetFrameLen(Frame f);
extern void WriteFrameToBuff(uint8_t *buff, Frame f);
extern void ConfDevCmd(Frame f);
extern void ConfDevFitter(Frame f);
extern void SendHeartBeat();
extern void SendSyncTime();
extern bool isFitter();
extern bool DiBangFitter(uint8_t const *pattern, uint8_t patternLen, uint8_t *data, uint8_t dataLen, uint8_t *matchcStart, uint8_t *matchEnd);
extern void DibangDataHandler();

#define ReqConfPeriod (10 * 1000) //这次等请求信息循环请求周期秒

#endif // !__PROTOCOL_H#define