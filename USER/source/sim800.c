#include <string.h>
#include "stm32f37x.h"
#include "sim800.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

char cSim800_Rsv;
char *pcSim800Rsv = &cSim800_Rsv;

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
    //A1配置为GPRS电源使能,A3配置为GPRS开关机
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP; //开漏
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_Initstruc);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1 | GPIO_Pin_3, Bit_RESET); //默认为低
    /*B8和B9配置为串口3,GPRS*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_7);
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_Initstruc);
    /*串口初始化*/
    UART_GPRS.Init();
}

/********************************************************************
* 功    能：SIM800电源使能
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_pEn(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
}
/********************************************************************
* 功    能：SIM800电源禁能
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_PDen(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
}

/********************************************************************
* 功    能：SIM800开关机
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_OnOff(void)
{
    /*引脚拉高*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    vTaskDelay(500 / portTICK_RATE_MS);
    /*引脚拉低*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_SET);
    /*延时1秒*/
    vTaskDelay(1000 / portTICK_RATE_MS);
    /*引脚拉高*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
}

/********************************************************************
* 功    能：SIM800复位
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_Reset(void)
{
}

/********************************************************************
* 功    能：SIM800发送命令
* 输    入：none
* 输    出：none
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
void vSim800_SndCmd(char *cmd)
{
    UART_GPRS.Send(&UART_GPRS, cmd, strlen(cmd)); // 末尾的\0 也要发送
}

/********************************************************************
* 功    能：SIM800接收命令
* 输    入：存放命令的字符串指针
* 输    出：接收到的字符数
* 编 写 人：stragen
* 编写日期：2018.3.27
**********************************************************************/
uint32_t Sim800_RsvCmd(char *cmd, uint32_t len, uint32_t delayMs)
{
    return UART_GPRS.Receive(&UART_GPRS, cmd, len, delayMs);
}
