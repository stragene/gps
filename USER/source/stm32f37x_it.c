/**
  ******************************************************************************
  * @file    Project/STM32F37x_StdPeriph_Templates/stm32f37x_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include "stdlib.h"
#include "stm32f37x.h"
#include "stm32f37x_it.h"
#include "main.h"
#include "uart.h"
#include "sim800.h"
#include "gps.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern SemaphoreHandle_t xSemGprsRsvd;
/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    //	if (CoreDebug->DHCSR & 1) {  //check C_DEBUGEN == 1 -> Debugger Connected
    //      __breakpoint(0);  // halt program execution here
    //  }

    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{

//}

/******************************************************************************/
/*                 STM32F37x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f37x.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/********************************************************************
* ��    �ܣ�GPS ����1���շ��� 
            ����������д��Uart1����ṹ���е�����
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�������
* ��д���ڣ�2016.6.23
**********************************************************************/
void USART1_IRQHandler(void)
{
    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) == SET)
        USART_ClearFlag(USART3, USART_FLAG_ORE);

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        if (pUartGPS->pRsvbuf->rd == (pUartGPS->pRsvbuf->wr + 1) % BUF_SIZE)
        {
            USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        }
        else
        {
            pUartGPS->pRsvbuf->data[pUartGPS->pRsvbuf->wr] = (uint8_t)USART_ReceiveData(USART1);
            pUartGPS->pRsvbuf->wr = (pUartGPS->pRsvbuf->wr + 1) % BUF_SIZE;
        }
    };

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
        if (pUartGPS->pSndbuf->rd == pUartGPS->pSndbuf->wr)
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
        else
        {
            USART_SendData(USART1, (uint16_t)pUartGPS->pSndbuf->data[pUartGPS->pSndbuf->rd]);
            pUartGPS->pSndbuf->rd = (pUartGPS->pSndbuf->rd + 1) % BUF_SIZE;
        }
    };
}

/********************************************************************
* ��    �ܣ�USART2_IRQHandler�жϺ���
            ���������ݶ�ȡ��Uart2����ṹ���е�����
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�������
* ��д���ڣ�2016.6.23
**********************************************************************/
void USART2_IRQHandler(void)
{
}

/********************************************************************
* ��    �ܣ�GPRS Receive
            ���������ݶ�ȡ��Uart3����ṹ���е�����
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�������
* ��д���ڣ�2016.6.23
**********************************************************************/
void USART3_IRQHandler(void)
{
    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE))
    {
        USART_ClearFlag(USART3, USART_FLAG_ORE);
    }
    if (RESET != USART_GetITStatus(USART3, USART_IT_RXNE)) //读一个字节入串口3接收缓存
    {
        if (uwBuf_EmpLen(pUartGPRS->pRsvbuf))
        {
            pUartGPRS->pRsvbuf->data[pUartGPRS->pRsvbuf->wr] = (uint8_t)USART_ReceiveData(USART3);
            pUartGPRS->pRsvbuf->wr = (++pUartGPRS->pRsvbuf->wr) % BUF_SIZE;
        }
    }
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_TXE);
        USART_SendData(USART3, (uint16_t)pUartGPRS->pSndbuf->data[pUartGPRS->pSndbuf->rd]);
        pUartGPRS->pSndbuf->rd = ++(pUartGPRS->pSndbuf->rd) & BUF_SIZE;
        if (uwBuf_UnReadLen(pUartGPRS->pSndbuf) == 0)
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    }
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);
        //FLAG_UartZD_HasData = true;
        xSemaphoreGiveFromISR(xSemGprsRsvd, vTaskRsvPPP);
    }
}

/********************************************************************
* ��    �ܣ�TIM2_IRQHandler�Ϻ���
            ��ʱ�ۼ�
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�������
* ��д���ڣ�2016.6.23
**********************************************************************/
void TIM2_IRQHandler(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
