#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "sim800.h"
//#include "gps.h"
//#include "time.h"

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"

//SemaphoreHandle_t xSemaphore_MeterReceived; //声明信号量句柄
//QueueHandle_t handQueueU1Frame;             //队列句柄
void vDelay_Us(uint32_t delay)
{
    uint32_t i;
    uint32_t j;
    for (i = 0; i < delay - 1; i++)
    {
        for (j = 0; j < 64; j++)
        {
            ;
        }
    }
}
void vDelay_Ms(uint32_t delay)
{
    uint32_t ms;
    uint32_t us;
    for (ms = 0; ms < delay - 1; ms++)
    {
        for (us = 0; us < 999; us++)
        {
            ;
        }
    }
}
void vSim800_TestInit(void)
{

    uint8_t cmd[3];
    /*电源使能*/
    vSim800_pEn();
    /*开机*/
    vDelay_Ms(500);
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    vDelay_Ms(500);
    /*引脚拉低*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_SET);
    /*延时1秒*/
    vDelay_Ms(1000);
    /*引脚拉高*/
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    //vSim800_OnOff();
    //vDelay_Ms(2000);
    //UART_GPRS.Sndbuf.buf[UART_GPRS.Sndbuf.wr] = 'A';
    //UART_GPRS.Sndbuf.wr++;
    //UART_GPRS.Sndbuf.buf[UART_GPRS.Sndbuf.wr] = 'T';
    //UART_GPRS.Sndbuf.wr++;
    //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    vDelay_Ms(2000);
    while (1)
        ;
}

int main(void)
{
    /*板级硬件初始化*/
    vBoardInit();
    vSim800_TestInit();

    //vSemaphoreCreateBinary(xSemaphore_MeterReceived);		//定义二进制信号量，收到电表侧内容
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));
    /*发起指示灯任务*/
    xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate((void *)vTaskComInit, "ComInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
}

void vBoardInit(void)
{
    /*配置中断向量组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /*内部Flash初始化*/
    vInnerFlash_Init();
    /*配置运行灯管脚*/
    vRunLed_Init();
    /*SIM800初始化*/
    vSim800_HardInit();
    /*GPS初始化*/
    vGps_Init();
    /*系统节拍初始化*/
    //vSysTickInit();
}

void vSim800_ComInit(void)
{
    char Cmd[10];
    char *pcmd = Cmd;
    uint32_t rsvLen = 0;
    uint32_t j = 5;
    /*电源使能*/
    vSim800_pEn();
    /*等待电源稳定*/
    vTaskDelay(1000 / portTICK_RATE_MS);
    /*开机*/
    vSim800_OnOff();
    /*延时2秒*/
    //vTaskDelay(2000 / portTICK_RATE_MS);
    rsvLen = Sim800_RsvCmd(pcmd, 10, 2000);
    while ((rsvLen != 6) && (j--))
    {
        vSim800_SndCmd("AT\r\n");
        rsvLen = Sim800_RsvCmd(pcmd, 10, 2000);
    }
    vTaskDelay(2000 / portTICK_RATE_MS);
    while (1)
        ;
}

void vTaskComInit(void)
{
    vSim800_ComInit();
}
