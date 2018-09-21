#include <string.h>
#include "stm32f37x.h"
#include "sim800.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

static void vSysTickInit_1ms(void);
static void vDelay_Ms(uint32_t delay);
static void vSim800_HardInit(void);
static void vSim800_pEn(void);
static void vSim800_PDen(void);
static void vSim800_OnOff(void);
static bool blSim800SendCmd(char *pcmd, char *response, uint32_t timeout, uint32_t retry);
static uint32_t dwSim800Send(uint8_t *pbuf, uint32_t len);
struct gprs_dev Sim800GPRS = {.Init = vSim800_HardInit,
                              .PowerEn = vSim800_pEn,
                              .PowerDen = vSim800_PDen,
                              .OnOff = vSim800_OnOff,
                              .SendCmd = blSim800SendCmd,
                              .Send = dwSim800Send,
                              .AutoReadEn = vSim800AutoReadEn,
                              .delay = vDelay_Ms};
struct gprs_dev *pSim800GPRS = &Sim800GPRS;

/********************************************************************
* 功    能：SIM800初始化
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_HardInit(void)
{
    GPIO_InitTypeDef GPIO_Initstruc;
    USART_InitTypeDef USART_Initstruc;
    NVIC_InitTypeDef NVIC_Initstruc;
    //A1配置为GPRS电源使能,A3配置为GPRS开关机
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP; //开漏
    //GPIO_Initstruc.GPIO_OType = GPIO_OType_OD; //开漏
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_WriteBit(GPIOA, GPIO_Pin_1 | GPIO_Pin_3, Bit_RESET); //默认为低
    GPIO_Init(GPIOA, &GPIO_Initstruc);
    /*B8和B9配置为串口3,GPRS*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_7);
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_UP;
    //GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_Initstruc);
    /*串口初始化*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_OverSampling8Cmd(USART3, ENABLE);
    USART_Initstruc.USART_BaudRate = 115200;
    USART_Initstruc.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
    USART_Initstruc.USART_WordLength = USART_WordLength_8b;
    USART_Initstruc.USART_Parity = USART_Parity_No;
    USART_Initstruc.USART_StopBits = USART_StopBits_1;
    USART_Initstruc.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3, &USART_Initstruc);

    NVIC_Initstruc.NVIC_IRQChannel = USART3_IRQn;
    //NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    //NVIC_Initstruc.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_Initstruc);
    USART_Cmd(USART3, ENABLE);

    vSysTickInit_1ms();
}

/********************************************************************
* 功    能：Sim800 Power On or Powen Off
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vSim800_OnOff(void)
{
    /*引脚拉高*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    vDelay_Ms(500);
    /*引脚拉低*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_SET);
    /*延时1秒*/
    vDelay_Ms(1000);
    /*引脚拉高*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
}

/****im***************************************************************
* 功    能：Sim800 Power enable
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vSim800_pEn(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
}
/********************************************************************
* 功    能：Sim800 Power Disable
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vSim800_PDen(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
}

/********************************************************************
* 功    能：Enable 1ms Systick , no interrupt
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vSysTickInit_1ms(void)
{
    uint32_t ticks = SystemCoreClock / 1000;
    SysTick->LOAD = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;       /* set reload register */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1); /* set Priority for Systick Interrupt */
    SysTick->VAL = 0;                                            /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    //SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
}

/********************************************************************
* 功    能：delayms
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vDelay_Ms(uint32_t ms)
{
    if (ms != 0xFFFFFFFF)
        ms++;
    while (ms)
    {
        if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
        {
            ms--;
        }
    }
}

/********************************************************************
* 功    能：vSim800SendCmd
* 输    入：
* 输    出：
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
bool blSim800SendCmd(char *pcmd, char *pExpectAns, uint32_t timeout, uint32_t retry)
{
    uint8_t buf[50];
    uint32_t readlen;
    bool result;
    char *pAns;
    do
    {
        Uart_OnceWrite(pUartGPRS, (uint8_t *)pcmd, strlen(pcmd), 500);
        readlen = Uart_OnceRead(pUartGPRS, &buf[0], 50, timeout);
        buf[readlen] = '\0';
        pAns = (char *)buf;
        if (pExpectAns == NULL)
        {
            if (0 == strcmp(pcmd, "AT+CSQ"))
            {
                result = ((buf[8] - '0') * 10 + (buf[9] - '0') >= 16);
            }
            else if (0 == strcmp(pcmd, "AT+CGATT?"))
            {
                result = ((buf[10] - '0') == 1);
            }
            else if (0 == strcmp(pcmd, "AT+CIFSR "))
            {
            }
        }
        else
        {
            pAns = strchr(pAns, *pExpectAns);
            result = !bcmp(pExpectAns, pAns, strlen(pExpectAns));
        }
        retry--;
    } while (!result && retry);
    return (result);
}
/********************************************************************
* 功    能：vSim800Send
* 输    入：
* 输    出：
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
uint32_t dwSim800Send(uint8_t *pbuf, uint32_t len)
{
    uint32_t sendlen;
    sendlen = Uart_OnceWrite(pUartGPRS, pbuf, len, 500);
}

/********************************************************************
* 功    能：vSim800Send
* 输    入：
* 输    出：
* 编 写 人：stragen
* 编写日期：
**********************************************************************/
void vSim800AutoReadEn(void)
{
    USART_ITConfig(pUartGPRS->handle, USART_IT_IDLE, ENABLE);
}