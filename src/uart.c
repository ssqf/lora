//串口相关处理

#include "uart.h"

const uint32_t LoraBaudRate = 115200; //lora WH-L101 默认值 波特率 115200、无校验、8 位数据位、1 位停止位
const uint32_t DevBaudRate = 9600;

bool IsLoraSend = FALSE;
bool IsDevSend = FALSE;
void SetRS485CTL(BitAction state);
static void initDMA();
static void initDeviceUart();
static void initLoraUart();
static void initRS458CTL();
void initUart3();

uint8_t LORA_RECV_BUFF[LORA_RECV_BUFF_SIZE];
uint8_t LORA_SEND_BUFF[LORA_SEND_BUFF_SIZE];
uint8_t DEV_RECV_BUFF[DEV_RECV_BUFF_SIZE];
uint8_t DEV_SEND_BUFF[DEV_SEND_BUFF_SIZE];
uint8_t LoraRecvPos = 0;
uint8_t DevRecvPos = 0;

void InitUart()
{
    initRS458CTL();
    initDeviceUart();
    initLoraUart();

#ifdef __DEBUG__
    initUart3();
#endif

    initDMA();
    USART_Cmd(DevCom, ENABLE);
    USART_Cmd(LoraCom, ENABLE);
}

static void initDMA()
{
    DMA_GlobalDeInit();
    DMA_DeInit(LORA_DMA_RX);
    DMA_DeInit(LORA_DMA_TX);
    DMA_DeInit(DEV_DMA_RX);
    DMA_DeInit(DEV_DMA_TX);

    //DMA要模式设置为Normal则传一次就结束，要重传要重新buff的地址和接收的数据
    DMA_Init(LORA_DMA_RX, LORA_RX_BUFF_ADDR, LORA_DR_ADDR, LORA_RECV_BUFF_SIZE, /* DMA_BufferSize */
             DMA_DIR_PeripheralToMemory, DMA_Mode_Circular, DMA_MemoryIncMode_Inc,
             DMA_Priority_High, DMA_MemoryDataSize_Byte);
    DMA_Init(LORA_DMA_TX, LORA_TX_BUFF_ADDR, LORA_DR_ADDR, 0, /* DMA_BufferSize */
             DMA_DIR_MemoryToPeripheral, DMA_Mode_Normal, DMA_MemoryIncMode_Inc,
             DMA_Priority_High, DMA_MemoryDataSize_Byte);

    DMA_Init(DEV_DMA_RX, DEV_RX_BUFF_ADDR, DEV_DR_ADDR, DEV_RECV_BUFF_SIZE, /* DMA_BufferSize */
             DMA_DIR_PeripheralToMemory, DMA_Mode_Circular, DMA_MemoryIncMode_Inc,
             DMA_Priority_Low, DMA_MemoryDataSize_Byte);
    DMA_Init(DEV_DMA_TX, DEV_TX_BUFF_ADDR, DEV_DR_ADDR, 0, /* DMA_BufferSize */
             DMA_DIR_MemoryToPeripheral, DMA_Mode_Normal, DMA_MemoryIncMode_Inc,
             DMA_Priority_Low, DMA_MemoryDataSize_Byte);

    DMA_ITConfig(LORA_DMA_RX, DMA_ITx_TC | DMA_ITx_HT, ENABLE);
    DMA_ITConfig(LORA_DMA_TX, DMA_ITx_TC, ENABLE);
    DMA_ITConfig(DEV_DMA_RX, DMA_ITx_TC | DMA_ITx_HT, ENABLE);
    DMA_ITConfig(DEV_DMA_TX, DMA_ITx_TC, ENABLE);

    DMA_Cmd(LORA_DMA_RX, ENABLE);
    DMA_Cmd(LORA_DMA_TX, ENABLE);
    DMA_Cmd(DEV_DMA_RX, ENABLE);
    DMA_Cmd(DEV_DMA_TX, ENABLE);

    DMA_GlobalCmd(ENABLE);

    USART_DMACmd(DevCom, USART_DMAReq_TX, ENABLE);
    USART_DMACmd(DevCom, USART_DMAReq_RX, ENABLE);
    USART_DMACmd(LoraCom, USART_DMAReq_TX, ENABLE);
    USART_DMACmd(LoraCom, USART_DMAReq_RX, ENABLE);
}

static void initDeviceUart()
{
    //USART_ClockInit(DevCom, USART_Clock_Enable, USART_CPOL_Low, USART_CPHA_1Edge, USART_LastBit_Disable);
    USART_SetPrescaler(DevCom, 1);

    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_3, ENABLE);
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_4, ENABLE);

    USART_Init(DevCom, conf.DevType ? 9600 : 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, USART_Mode_Rx | USART_Mode_Tx);
    //USART_Init(DevCom, DevBaudRate, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, USART_Mode_Rx);
    //USART_DMACmd(DevCom, USART_DMAReq_TX, ENABLE);
    //USART_DMACmd(DevCom, USART_DMAReq_RX, ENABLE);
    USART_ITConfig(DevCom, USART_IT_IDLE, ENABLE);
    //USART_ITConfig(DevCom, USART_IT_TC, ENABLE);
    USART_Cmd(DevCom, DISABLE);
}

void ReOpenDevUart(uint32_t BaudRate, USART_WordLength_TypeDef USART_WordLength, USART_StopBits_TypeDef USART_StopBits,
                   USART_Parity_TypeDef USART_Parity)
{
    USART_Cmd(DevCom, DISABLE);
    USART_Init(DevCom, BaudRate, USART_WordLength, USART_StopBits, USART_Parity, USART_Mode_Rx | USART_Mode_Tx);
    USART_Cmd(DevCom, ENABLE);
}

void ReOpenLoraUart(uint32_t BaudRate, USART_WordLength_TypeDef USART_WordLength, USART_StopBits_TypeDef USART_StopBits,
                    USART_Parity_TypeDef USART_Parity)
{
    USART_Cmd(LoraCom, DISABLE);
    USART_Init(LoraCom, BaudRate, USART_WordLength, USART_StopBits, USART_Parity, USART_Mode_Rx | USART_Mode_Tx);
    USART_Cmd(LoraCom, ENABLE);
}

static void initLoraUart()
{
    //USART_ClockInit(LoraCom, USART_Clock_Enable, USART_CPOL_Low, USART_CPHA_1Edge, USART_LastBit_Disable);
    USART_SetPrescaler(LoraCom, 1);

    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE);
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE);

    USART_Init(LoraCom, LoraBaudRate, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, USART_Mode_Rx | USART_Mode_Tx);
    //USART_DMACmd(LoraCom, USART_DMAReq_TX, ENABLE);
    //USART_DMACmd(LoraCom, USART_DMAReq_RX, ENABLE);
    USART_ITConfig(LoraCom, USART_IT_IDLE, ENABLE);
    //USART_ITConfig(LoraCom, USART_IT_TC, ENABLE);
    USART_Cmd(DevCom, DISABLE);
}

void SendDevice(uint8_t *data, uint8_t dataLen)
{
    uint8_t len = 0;
    uint8_t pos = 0;
    uint8_t remainLen = dataLen;
    while (0 != remainLen)
    {
        while (USART_GetFlagStatus(DevCom, USART_FLAG_TC) != SET) //DMA 完成不等于串口发送完成，要等待串口发送完成，不然丢数据
            ;

        len = DEV_SEND_BUFF_SIZE <= remainLen ? DEV_SEND_BUFF_SIZE : remainLen;
        memcpy(DEV_SEND_BUFF, data + pos, len);
        DMA_Cmd(DEV_DMA_TX, DISABLE);
        DMA_SetCurrDataCounter(DEV_DMA_TX, len);
        SetRS485CTL(SET);
        DMA_Cmd(DEV_DMA_TX, ENABLE);
        //USART_DMACmd(DevCom, USART_DMAReq_TX, ENABLE);
        Debug("SendDevice len:%d", len);
        pos = pos + len;
        remainLen = remainLen - len;
    }
}

void SendLora(uint8_t *data, uint8_t dataLen)
{
    uint8_t len = 0;
    uint8_t pos = 0;
    uint8_t remainLen = dataLen;
    while (0 != remainLen)
    {
        while (USART_GetFlagStatus(LoraCom, USART_FLAG_TC) != SET)
            ; //DMA 完成不等于串口发送完成，要等待串口发送完成，不然丢数据
        SetWakeState(RESET);
        DelayUs(6000);
        SetWakeState(SET);
        len = LORA_SEND_BUFF_SIZE <= remainLen ? LORA_SEND_BUFF_SIZE : remainLen;
        memcpy(LORA_SEND_BUFF, data + pos, len);
        DMA_Cmd(LORA_DMA_TX, DISABLE);
        DMA_SetCurrDataCounter(LORA_DMA_TX, len);
        DMA_Cmd(LORA_DMA_TX, ENABLE);
        Debug("SendLora dataLen:%d len:%d", dataLen, len);
        //USART_DMACmd(LoraCom, USART_DMAReq_TX, ENABLE);
        pos = pos + len;
        remainLen = remainLen - len;
    }
}

static void initRS458CTL()
{
    GPIO_Init(GPIOE, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast);
    SetRS485CTL(RESET);
}

void SetRS485CTL(BitAction state)
{
    GPIO_WriteBit(GPIOE, GPIO_Pin_2, state);
}

void SetLoraReadySend()
{
    SetWakeState(RESET);
    //Delay5ms();
    DelaySendTask(5, LORA_DATA_SEND);
}

void initUart3()
{
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART3TxRxPortF, ENABLE);
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOF, GPIO_Pin_0, ENABLE);

    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOF, GPIO_Pin_1, ENABLE);

    //USART_SetPrescaler(USART3, 1);

    USART_Init(USART3, 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, USART_Mode_Rx | USART_Mode_Tx);
    //USART_ITConfig(USART3, , ENABLE);
    USART_Cmd(USART3, ENABLE);
}

void ResetDevRx()
{
    DMA_Cmd(DEV_DMA_RX, DISABLE);
    DMA_SetCurrDataCounter(DEV_DMA_RX, DEV_RECV_BUFF_SIZE);
    DMA_Cmd(DEV_DMA_RX, ENABLE);
    DevCom->SR;
    DevCom->DR;
    //DMA_ClearFlag(DEV_DMA_FLAG_TCRX);
    //USART_ClearFlag(DevCom, USART_FLAG_IDLE);
}

void ResetLoraRx()
{
    DMA_Cmd(LORA_DMA_RX, DISABLE);
    DMA_SetCurrDataCounter(LORA_DMA_RX, LORA_RECV_BUFF_SIZE);
    DMA_Cmd(LORA_DMA_RX, ENABLE);
    LoraCom->SR;
    LoraCom->DR;
    //DMA_ClearFlag(LORA_DMA_FLAG_TCRX);
    //USART_ClearFlag(LoraCom, USART_FLAG_IDLE);
}

int putchar(int c)
{
    /* Write a character to the USART */
    USART_SendData8(USART3, c);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        ;

    return (c);
}

int getchar(void)
{
    int c = 0;
    /* Loop until the Read data register flag is SET */
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
        ;
    c = USART_ReceiveData8(USART3);
    return (c);
}

void ShowString(char *str)
{
    while ('\0' != *str)
    {
        putchar(*str++);
    }
}

//BUG:包比较大的时候就卡主了 49以下可以，超过49则死掉了,原因是一直进中断了-OK
//BUG:缓存满了之后紧跟的数据会掉