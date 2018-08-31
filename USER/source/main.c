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

//SemaphoreHandle_t xSemaphore_MeterReceived; //�����ź������
//QueueHandle_t handQueueU1Frame;             //���о��
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
    /*��Դʹ��*/
    vSim800_pEn();
    /*����*/
    vDelay_Ms(500);
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    vDelay_Ms(500);
    /*��������*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_SET);
    /*��ʱ1��*/
    vDelay_Ms(1000);
    /*��������*/
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
    /*�弶Ӳ����ʼ��*/
    vBoardInit();
    vSim800_TestInit();

    //vSemaphoreCreateBinary(xSemaphore_MeterReceived);		//����������ź������յ���������
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));
    /*����ָʾ������*/
    xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate((void *)vTaskComInit, "ComInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
}

void vBoardInit(void)
{
    /*�����ж�������*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /*�ڲ�Flash��ʼ��*/
    vInnerFlash_Init();
    /*�������еƹܽ�*/
    vRunLed_Init();
    /*SIM800��ʼ��*/
    vSim800_HardInit();
    /*GPS��ʼ��*/
    vGps_Init();
    /*ϵͳ���ĳ�ʼ��*/
    //vSysTickInit();
}

void vSim800_ComInit(void)
{
    char Cmd[10];
    char *pcmd = Cmd;
    uint32_t rsvLen = 0;
    uint32_t j = 5;
    /*��Դʹ��*/
    vSim800_pEn();
    /*�ȴ���Դ�ȶ�*/
    vTaskDelay(1000 / portTICK_RATE_MS);
    /*����*/
    vSim800_OnOff();
    /*��ʱ2��*/
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
