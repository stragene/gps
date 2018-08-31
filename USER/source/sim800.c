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

/********************************************************************
* ��    �ܣ�SIM800��Դʹ��
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_pEn(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
}
/********************************************************************
* ��    �ܣ�SIM800��Դ����
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_PDen(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
}

/********************************************************************
* ��    �ܣ�SIM800���ػ�
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
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
* ��    �ܣ�SIM800��λ
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_Reset(void)
{
}

/********************************************************************
* ��    �ܣ�SIM800��������
* ��    �룺none
* ��    ����none
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
void vSim800_SndCmd(char *cmd)
{
    UART_GPRS.Send(&UART_GPRS, cmd, strlen(cmd)); // ĩβ��\0 ҲҪ����
}

/********************************************************************
* ��    �ܣ�SIM800��������
* ��    �룺���������ַ���ָ��
* ��    �������յ����ַ���
* �� д �ˣ�stragen
* ��д���ڣ�2018.3.27
**********************************************************************/
uint32_t Sim800_RsvCmd(char *cmd, uint32_t len, uint32_t delayMs)
{
    return UART_GPRS.Receive(&UART_GPRS, cmd, len, delayMs);
}
