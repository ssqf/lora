// 初始化设备
#include <lora.h>
void initCLK();
void InitDevice()
{
    initCLK();
    InitLoraPin();
    InitConf();
    InitUart();
    InitClock();
    InitTask();
    enableInterrupts();

    if (RESET == GetConfFlag(Conf_FLAG_INTEGRITY)) //数据不完成则恢复出厂设置
    {
        ConfRestoredefault();
    }

    if (Conf.DevType == DevType_UnKnown) //设备类型未知
    {
        InitGPRSConf(); //尝试设置GPRS，并根据条件判断是不是网关
        //GwRegister();   //如果可以注册成功就认为是网关,不能同时进行
    }
    else
    {
        if (RESET == GetConfFlag(Conf_FLAG_LORA)) //配置lora参数
        {
            InitLoraConf();
        }

        if (Conf.DevType == DevType_LoRa_Node)
        {

            if (RESET == GetConfFlag(Conf_FLAG_LORA_REGIST)) //lora配置成功，但未注册成功
            {
                LoraRegister();
            }
            else if (RESET == GetConfFlag(Conf_FLAG_LORA_DEV_Done)) //Lora对应的设备参数未配置成功
            {
                GetLoraDevConf();
            }
        }
        else if (Conf.DevType == DevType_GW)
        {
            if (RESET == GetConfFlag(Conf_FLAG_GPRS))
            {
                InitGPRSConf();
            }
        }
    }
    SendSyncTime();
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
        ;           //等待切换到外部晶振
    DelayUs(20000); //时钟初始化后可能不稳定，要加延时

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
}

void DelayUs(uint32_t delay)
{
    uint32_t d = delay * 16;
    for (; d; d--)
        ;
}
