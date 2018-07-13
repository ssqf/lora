// 初始化设备
#include <lora.h>
void initCLK();
void InitDevice()
{
    initCLK();
    InitUart();
    InitClock();
    InitTask();
    enableInterrupts();
}

void initCLK()
{
    //配置CPU时钟位外部时钟不分频16M
    CLK_DeInit();
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    CLK_HSEConfig(CLK_HSE_ON);
    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);

    while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE)
        ; //等待切换到外部晶振

    CLK_RTCClockConfig(CLK_RTCCLKSource_HSE, CLK_RTCCLKDiv_64); //16MHz/64=25KHz

    // 使能TIM1 TIM2 uart1 uart2 和TRC
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART2, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART3, ENABLE); //串口3做调试
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);
}

void initMapPort()
{
    SYSCFG_REMAPDeInit();
    //SYSCFG_REMAPPinConfig();
}
