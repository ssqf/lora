#include "stm8l15x_it.h"
#include "lora.h"

#ifdef _COSMIC_
/**
  * @brief Dummy interrupt routine
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(NonHandledInterrupt, 0)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
#endif

INTERRUPT_HANDLER_TRAP(TRAP_IRQHandler)
{
}

INTERRUPT_HANDLER(FLASH_IRQHandler, 1)
{
}

INTERRUPT_HANDLER(DMA1_CHANNEL0_1_IRQHandler, 2)
{

  if (DMA_GetFlagStatus(DMA1_FLAG_TC0) != RESET)
  {
    PushTask(DEV_SEND_COMPLETE);
    DMA_ClearITPendingBit(DMA1_IT_TC0);
  }
  if (DMA_GetFlagStatus(DMA1_FLAG_TC1) != RESET)
  {
    PushTask(LORA_SEND_COMPLETE);
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}

// DMA 通道23的中断，做串号收数据
INTERRUPT_HANDLER(DMA1_CHANNEL2_3_IRQHandler, 3)
{
  //LORA DMA 接收数据一半
  if (DMA_GetFlagStatus(DMA1_FLAG_HT2) != RESET)
  {
    uint8_t currPos = LORA_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(LORA_DMA_RX);
    uint8_t len = currPos - LoraRecvPos;
    bool rst = PushDataLoraBuff(&LORA_RECV_BUFF[LoraRecvPos], len);
    PushTask(LORA_RECV_DATA);
    LoraRecvPos = currPos;
    DMA_ClearFlag(DMA1_FLAG_HT2);
    Debug("Lora HF Rx:%d push rst:%d currPos:%d LoraRecvPos:%d", len, rst, currPos, LoraRecvPos);
  }

  //LORA DMA 接收数据完成
  if (DMA_GetFlagStatus(LORA_DMA_FLAG_TCRX) != RESET)
  {
    uint8_t len = LORA_RECV_BUFF_SIZE - LoraRecvPos;
    bool rst = PushDataLoraBuff(&LORA_RECV_BUFF[LoraRecvPos], len);
    PushTask(LORA_RECV_DATA);
    LoraRecvPos = 0;
    DMA_ClearFlag(LORA_DMA_FLAG_TCRX);
    Debug("Lora TC Rx:%d push rst:%d currPos:%d LoraRecvPos:%d", len, rst, LORA_RECV_BUFF_SIZE, LoraRecvPos);
  }

  //Dev DMA 接收数据一半
  if (DMA_GetFlagStatus(DMA1_FLAG_HT3) != RESET)
  {
    uint8_t currPos = DEV_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(DEV_DMA_RX);
    uint8_t len = currPos - DevRecvPos;
    bool rst = PushDataDevBuff(&DEV_RECV_BUFF[DevRecvPos], len);
    PushTask(DEV_RECV_DATA);
    DevRecvPos = currPos;
    DMA_ClearFlag(DMA1_FLAG_HT3);
    Debug("Dev HF Rx:%d push rst:%d currPos:%d DevRecvPos:%d", len, rst, currPos, DevRecvPos);
  }

  //DEV DMA 接收数据完成
  if (DMA_GetFlagStatus(DEV_DMA_FLAG_TCRX) != RESET)
  {
    uint8_t len = DEV_RECV_BUFF_SIZE - DevRecvPos;
    bool rst = PushDataDevBuff(&DEV_RECV_BUFF[DevRecvPos], len);
    PushTask(DEV_RECV_DATA);
    DevRecvPos = 0;
    DMA_ClearFlag(DEV_DMA_FLAG_TCRX);
    Debug("Dev TC Rx:%d push rst:%d currPos:%d DevRecvPos:%d", len, rst, DEV_RECV_BUFF_SIZE, DevRecvPos);
  }
}

INTERRUPT_HANDLER(RTC_CSSLSE_IRQHandler, 4)
{
}

INTERRUPT_HANDLER(EXTIE_F_PVD_IRQHandler, 5)
{
}

INTERRUPT_HANDLER(EXTIB_G_IRQHandler, 6)
{
}

INTERRUPT_HANDLER(EXTID_H_IRQHandler, 7)
{
}

INTERRUPT_HANDLER(EXTI0_IRQHandler, 8)
{
}

INTERRUPT_HANDLER(EXTI1_IRQHandler, 9)
{
}

INTERRUPT_HANDLER(EXTI2_IRQHandler, 10)
{
}

INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)
{
}

INTERRUPT_HANDLER(EXTI4_IRQHandler, 12)
{
}

INTERRUPT_HANDLER(EXTI5_IRQHandler, 13)
{
}

INTERRUPT_HANDLER(EXTI6_IRQHandler, 14)
{
}

INTERRUPT_HANDLER(EXTI7_IRQHandler, 15)
{
}

INTERRUPT_HANDLER(LCD_AES_IRQHandler, 16)
{
}

INTERRUPT_HANDLER(SWITCH_CSS_BREAK_DAC_IRQHandler, 17)
{
}

INTERRUPT_HANDLER(ADC1_COMP_IRQHandler, 18)
{
}

INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler, 19)
{
  if (TIM2_GetITStatus(TIM2_IT_Update) == SET)
  {
    TIM2_ClearITPendingBit(TIM2_IT_Update);
    TickNum++;
    if (TickNum >= 50)
    {
      TickNum = 0;
      Second++;
    }
    TaskType t = TICK;
    PushTask(t);
    //TIM2_ClearFlag(TIM2_FLAG_Update);
  }
}

//Dev 串口空闲中断检测
INTERRUPT_HANDLER(TIM2_CC_USART2_RX_IRQHandler, 20)
{
  if (USART_GetFlagStatus(DevCom, USART_FLAG_IDLE))
  {
    uint8_t curPos = DEV_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(DEV_DMA_RX);
    uint8_t len = curPos - DevRecvPos;
    bool rst;
    if (len != 0)
    {
      rst = PushDataDevBuff(&DEV_RECV_BUFF[DevRecvPos], len);
      PushTask(DEV_RECV_DATA);
    }
    DevRecvPos = curPos;
    DevCom->SR;
    DevCom->DR;
    USART_ClearFlag(DevCom, USART_FLAG_IDLE);
    Debug("Dev idle Rx:%d push rst:%d curPos:%d DevRecvPos:%d", len, rst, curPos, DevRecvPos);
  }
  // if (USART_GetFlagStatus(DevCom, USART_FLAG_IDLE))
  // {
  //   uint8_t len = DEV_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(DEV_DMA_RX);
  //   bool rst = PushDataDevBuff(&DEV_RECV_BUFF[DevRecvPos], len);
  //   PushTask(DEV_RECV_DATA);
  //   ResetDevRx();
  //   Debug("Dev idle Rx:%d push rst:%d", len, rst);
  // }
}

INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_USART3_TX_IRQHandler, 21)
{
}

INTERRUPT_HANDLER(TIM3_CC_USART3_RX_IRQHandler, 22)
{
}

INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_COM_IRQHandler, 23)
{

  if (TIM1_GetITStatus(TIM1_IT_Update) == SET)
  {
    TIM1_ClearITPendingBit(TIM1_IT_Update);
    TIM1_ClearFlag(TIM1_FLAG_Update);
    TIM1_Cmd(DISABLE);

    DelayTask *t = GetCurrTask();
    if (NULL != t)
    {
      PushTask(t->t);
      t->used = FALSE;
      Debug("Timer1 Push:%d", t->t);
    }
    else
    {
      Debug("Timer1 Push NULL");
    }
    DelayTask *minTask = GetMinTask();
    if (NULL != minTask)
    {

      StartNextDelyaTask(minTask->delay);
      Debug("Next delay %d task:%d", minTask->delay, minTask->t);
    }
    else
    {
      Debug("Next task is NULL");
    }
  }
}

INTERRUPT_HANDLER(TIM1_CC_IRQHandler, 24)
{
}

INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler, 25)
{
}

INTERRUPT_HANDLER(SPI1_IRQHandler, 26)
{
}

INTERRUPT_HANDLER(USART1_TX_TIM5_UPD_OVF_TRG_BRK_IRQHandler, 27)
{
}

// lora 空闲中断
INTERRUPT_HANDLER(USART1_RX_TIM5_CC_IRQHandler, 28)
{
  if (USART_GetFlagStatus(LoraCom, USART_FLAG_IDLE))
  {
    uint8_t curPos = LORA_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(LORA_DMA_RX);
    uint8_t len = curPos - LoraRecvPos;
    bool rst ;
    if (len != 0)
    {
      rst = PushDataLoraBuff(&LORA_RECV_BUFF[LoraRecvPos], len);
      PushTask(LORA_RECV_DATA);
    }
    LoraRecvPos = curPos;
    LoraCom->SR;
    LoraCom->DR;
    USART_ClearFlag(LoraCom, USART_FLAG_IDLE);
    Debug("Lora idle Rx:%d push rst:%d curPos:%d LoraRecvPos:%d", len, rst, curPos, LoraRecvPos);
  }
  // if (USART_GetFlagStatus(LoraCom, USART_FLAG_IDLE))
  // {
  //   uint8_t len = LORA_RECV_BUFF_SIZE - DMA_GetCurrDataCounter(LORA_DMA_RX);
  //   bool rst = PushDataLoraBuff(&LORA_RECV_BUFF[LoraRecvPos], len);
  //   PushTask(LORA_RECV_DATA);
  //   ResetLoraRx();
  //   Debug("Lora idle Rx:%d push rst:%d", len, rst);
  // }
}

INTERRUPT_HANDLER(I2C1_SPI2_IRQHandler, 29)
{
}
