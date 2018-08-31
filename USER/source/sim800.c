#include <string.h>
#include "stm32f37x.h"
#include "sim800.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

char cSim800_Rsv;
char *pcSim800Rsv = &cSim800_Rsv;

/********************************************************************
* ��    �ܣ�SIM800��ʼ��
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_HardInit(void)
{
    GPIO_InitTypeDef GPIO_Initstruc;
    //A1����ΪGPRS��Դʹ��,A3����ΪGPRS���ػ�
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP; //��©
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_Initstruc);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1 | GPIO_Pin_3, Bit_RESET); //Ĭ��Ϊ��
    /*B8��B9����Ϊ����3,GPRS*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_7);
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_Initstruc);
    /*���ڳ�ʼ��*/
    UART_GPRS.Init();
}

void vSim800_OnOff(void)
{
    /*��������*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
    vTaskDelay(500 / portTICK_RATE_MS);
    /*��������*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_SET);
    /*��ʱ1��*/
    vTaskDelay(1000 / portTICK_RATE_MS);
    /*��������*/
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, Bit_RESET);
}

/********************************************************************
* ��    �ܣ�SIM800��������
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_SndCmd(UART_TypeDef *puart, char *cmd)
{
    puart->Send(puart, cmd, strlen(cmd)); // ĩβ��\0 ҲҪ����
}

/********************************************************************
* ��    �ܣ�SIM800��������
* ��    �룺���������ַ���ָ��
* ��    �������յ����ַ���
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
uint32_t Sim800_RsvCmd(UART_TypeDef *puart, char *cmd, uint32_t len, uint32_t delayMs)
{
    return puart->Receive(puart, (uint8_t *)cmd, len, delayMs);
}
