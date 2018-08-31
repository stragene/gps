/*********************************************************************** 
* Copyright (c) 2012,积成电子股份有限公司 All rights reserved.
*
* 文件名称： UART.c
* 描    述： 串口通信代码文件
* 
* 修改记录：
*
* V1.0，2016.06.19，
***********************************************************************/

#include <string.h>
#include "uart.h"
#include "stm32f37x.h"
#include "stm32f37x_it.h"
#include "FreeRTOS.h"
#include "task.h"

UART_TypeDef UART_GPS = {.hard = USART1,
                         .Init = vUart_GPS_Init,
                         .Send = vUart_Send,
                         .Receive = udwUart_Receive};
UART_TypeDef UART_GPRS = {.hard = USART3,
                          .Init = vUart_GPRS_Init,
                          .Send = vUart_Send,
                          .Receive = udwUart_Receive};

/********************************************************************
* 功    能：GPS串口初始化函数
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2016.6.23
**********************************************************************/
void vUart_GPS_Init(void)
{
    USART_InitTypeDef USART_Initstruc;
    NVIC_InitTypeDef NVIC_Initstruc;

    /*USART1配置 gps*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_OverSampling8Cmd(USART1, ENABLE);
    USART_Initstruc.USART_BaudRate = 115200;
    USART_Initstruc.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
    USART_Initstruc.USART_WordLength = USART_WordLength_9b;
    USART_Initstruc.USART_Parity = USART_Parity_No;
    USART_Initstruc.USART_StopBits = USART_StopBits_1;
    USART_Initstruc.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_Initstruc);

    /*配置串口1中断*/

    NVIC_Initstruc.NVIC_IRQChannel = USART1_IRQn;
    NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_Initstruc.NVIC_IRQChannelSubPriority = 0;
    NVIC_Initstruc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_Initstruc);

    USART_Cmd(USART3, ENABLE);
}

/********************************************************************
* 功    能：GPS串口初始化函数
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2016.6.23
**********************************************************************/
void vUart_GPRS_Init(void)
{
    USART_InitTypeDef USART_Initstruc;
    NVIC_InitTypeDef NVIC_Initstruc;

    /*USART3配置GPRS*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_OverSampling8Cmd(USART3, ENABLE);
    USART_Initstruc.USART_BaudRate = 115200;
    USART_Initstruc.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
    USART_Initstruc.USART_WordLength = USART_WordLength_9b;
    USART_Initstruc.USART_Parity = USART_Parity_No;
    USART_Initstruc.USART_StopBits = USART_StopBits_1;
    USART_Initstruc.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3, &USART_Initstruc);

    /*配置串口3中断*/
    NVIC_Initstruc.NVIC_IRQChannel = USART3_IRQn;
    NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_Initstruc.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_Initstruc);

    USART_Cmd(USART3, ENABLE);
}

/********************************************************************
* 功    能：GPS串口发送
* 输    入：要发送打字符串
* 输    出：
*           
* 编 写 人：
* 编写日期：2018.2.28
**********************************************************************/
void vUart_Send(UART_TypeDef *puart, uint8_t *pstring, uint8_t len)
{
    uint8_t i;
    if (ucDrvBuf_EmpLen(&(puart->Sndbuf)) >= len)
    {
        for (i = 0; i < len; i++)
        {
            puart->Sndbuf.buf[puart->Sndbuf.wr] = *(pstring + i);
            puart->Sndbuf.wr = (puart->Sndbuf.wr + 1) % DRV_BUF_SIZE;
        }
        USART_ITConfig(puart->hard, USART_IT_TXE, ENABLE);
    }
    else
        return;
}

/********************************************************************
* 功    能：GPS串口接收
* 输    入：
* 输    出：
*           
* 编 写 人：
* 编写日期：2018.2.28
**********************************************************************/
uint32_t udwUart_Receive(UART_TypeDef *puart, uint8_t *pstring, uint8_t len, uint32_t waitMs)
{
    uint32_t i = 0;
    uint32_t readed_len = 0;
    USART_ITConfig(puart->hard, USART_IT_RXNE, ENABLE);
    vTaskDelay(waitMs / portTICK_RATE_MS); //超时退出
    USART_ITConfig(puart->hard, USART_IT_RXNE, DISABLE);
    /*缓存中未读长度*/
    readed_len = ucDrvBuf_DataLen(&(puart->Rsvbuf));
    if (readed_len >= len)
    {
        readed_len = len;
    }
    for (i = 0; i < readed_len; i++)
    {
        *(pstring + i) = puart->Rsvbuf.buf[puart->Rsvbuf.rd];
        puart->Rsvbuf.rd = (puart->Rsvbuf.rd + 1) % DRV_BUF_SIZE;
    }
    return readed_len;
}

/********************************************************************
* 功    能：查询驱动层缓存剩余空间
* 输    入：数据缓存结构体地址
* 输    出：uint16_t
*           
* 编 写 人：
* 编写日期：2016.6.28
**********************************************************************/
uint16_t ucDrvBuf_EmpLen(const UCDRV_BUF *ucdrv_buf)
{
    uint16_t i = (ucdrv_buf->rd - ucdrv_buf->wr) % DRV_BUF_SIZE - 1;

    return i;
}

/********************************************************************
* 功    能：查询缓存中未读的字节数
* 输    入：数据缓存结构体地址
* 输    出：uint16_t
*          
* 编 写 人：
* 编写日期：2016.6.28
**********************************************************************/
uint16_t ucDrvBuf_DataLen(const UCDRV_BUF *ucdrv_buf)
{
    return (ucdrv_buf->wr - ucdrv_buf->rd + DRV_BUF_SIZE) % DRV_BUF_SIZE;
}
/********************************************************************
* 功    能：查找缓存中特定字符
* 输    入：缓存结构体指针uc_buf，特定字符chr,缓存内字节数ln
* 输    出：true/false
* 说    明：读指针指向该字符           
* 编 写 人：stragen
* 编写日期：2016.8.25
**********************************************************************/
bool Bufchr(UCDRV_BUF *uc_buf, uint8_t chr, uint16_t ln)
{
    while (ln-- && (uc_buf->buf[uc_buf->rd] != chr)) // &&(uc_buf->RD_Index != uc_buf->WR_Index))
    {
        uc_buf->rd = (uc_buf->rd + 1) % DRV_BUF_SIZE;
    }
    //return(ln ? true : false);						//读指针可能是0
    if (ln)
        return false;
    else
        return true;
}

/********************************************************************
* 功    能：判断缓存中字符串是否是特定字符串
* 输    入：缓存结构体地址uc_buf，字符串指针ptchr，字符串长度ln
* 输    出：true/false
* 说    明：读指针不改变
* 编 写 人：stragen
* 编写日期：2016.8.25
**********************************************************************/
bool Buf_cmp(UCDRV_BUF *uc_buf, char *ptchr, uint8_t ln)
{
    while (ln-- && (uc_buf->buf[uc_buf->rd] == *(ptchr++)) && (uc_buf->rd != uc_buf->wr)) //防止读取越界
    {
        uc_buf->rd = (uc_buf->rd + 1) % DRV_BUF_SIZE;
    }
    return (ln ? false : true);
}
