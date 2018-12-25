#include <stdlib.h>
#include <string.h>
#include "stm32f37x.h"
#include "gps.h"
#include "uart.h"

static void vGps_Init(void);
static void vGps_PEn(void);
static void vGps_PDen(void);
static char *pGetGprmc(uint8_t *buf, uint8_t len);
static bool GprmcFill(char *pframe, struct frame_gprmc *tmpGprmc);
/* 结构体定义-- ------------------------------------------------------------------*/

struct gps_dev Sim800GPS = {.Init = vGps_Init,
                            .PowerEn = vGps_PEn,
                            .PowerDen = vGps_PDen,
                            .Read = Uart_OnceRead,
                            .pGetGprmc = pGetGprmc,
                            .GprmcFill = GprmcFill};

struct gps_dev *pSim800GPS = &Sim800GPS;

/*******************************************************************************
* 函数名称           : vGps_Init()
* 函数功能           : GPS外设初始化
* 输    入           : 
* 输    出           : 
* 返 回 值           : 1
* 备    注           :                       
*******************************************************************************/
void vGps_Init(void)
{
    /*PA2配置为GPS使能*/
    GPIO_InitTypeDef GPIO_Initstruc;
    USART_InitTypeDef USART_Initstruc;
    NVIC_InitTypeDef NVIC_Initstruc;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_2;
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_Initstruc);
    /*B6和B7配置为串口1,GPS*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_7);
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_Initstruc);
    /*串口初始化*/

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_OverSampling8Cmd(USART1, ENABLE);
    USART_Initstruc.USART_BaudRate = 115200;
    USART_Initstruc.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
    USART_Initstruc.USART_WordLength = USART_WordLength_9b;
    USART_Initstruc.USART_Parity = USART_Parity_No;
    USART_Initstruc.USART_StopBits = USART_StopBits_1;
    USART_Initstruc.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_Initstruc);

    NVIC_Initstruc.NVIC_IRQChannel = USART1_IRQn;
    NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_Initstruc.NVIC_IRQChannelSubPriority = 0;
    NVIC_Initstruc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_Initstruc);

    USART_Cmd(USART1, ENABLE);
    //UART_GPS.Init();
}

/*******************************************************************************
* 函数名称           : vGps_EnDen()
* 函数功能           : GPS使能/禁能
* 输    入           : 
* 输    出           : 
* 返 回 值           : 1
* 备    注           :                       
*******************************************************************************/
void vGps_PEn(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET);
}
/*******************************************************************************
* 函数名称           : vGps_EnDen()
* 函数功能           : GPS使能/禁能
* 输    入           : 
* 输    出           : 
* 返 回 值           : 1
* 备    注           :                       
*******************************************************************************/
void vGps_PDen(void)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET);
}

/*******************************************************************************
* 函数名称           : 读取Gprmc帧
* 函数功能           : 
* 输    入           : 
* 输    出           : 
* 返 回 值           : 1
* 备    注           :                       
*******************************************************************************/
char *pGetGprmc(uint8_t *buf, uint8_t len)
{
    //char ucGPSFrame[DRV_BUF_SIZE];
    *(buf + len) = '\0';
    char *pGPSFrame = (char *)buf;
    /*找到$GPRMC*/
    do
    {
        pGPSFrame = strchr(pGPSFrame, '$');
    } while (strncmp(pGPSFrame, "$GPRMC", 6) != 0);
    return pGPSFrame;
}
/*******************************************************************************
* 函数名称           : GPS_Analysis()
* 函数功能           : 对GPS数据帧进行解析并装入到结构体中
* 输    入           : 
* 输    出           : 
* 返 回 值           : 1
* 备    注           :                       
*******************************************************************************/
bool GprmcFill(char *pframe, struct frame_gprmc *tmpGprmc)
{
    uint32_t j = 0;
    char *tmpchr[14];
    char *pGPSFrame;
    bool result = false;

    /*
	tmpchr[0]= strtok( pGPSFrame, ","); 	
	for( j= 1; j< 13; j++)
	{
		tmpchr[j]= strtok( NULL, ",");
		if (tmpchr[j] != NULL)
			continue;
		else
			break;
	}*/

    /*子串化并转存*/
    tmpchr[0] = strtok(pframe, ",");
    while (tmpchr[j] != NULL && j < 12) //以此存储第1~12子帧，第12帧以*结尾
    {
        tmpchr[++j] = strtok(NULL, ",");
    }

    pGPSFrame = tmpchr[12];              // mode*sum
    tmpchr[12] = strtok(pGPSFrame, "*"); // mode
    tmpchr[13] = strtok(NULL, "*");      // sum
    if (j != 12)
    {
        result = false;
    }
    else
    {
        if (!strncmp(tmpchr[12], "V", 1))
        {
            result = false;
        }

        else
        {
            tmpGprmc->UTC = *tmpchr[1];
            tmpGprmc->VA = *tmpchr[2];
            tmpGprmc->WD = atof(tmpchr[3]);
            tmpGprmc->WDNS = *tmpchr[4];
            tmpGprmc->JD = atof(tmpchr[5]);
            tmpGprmc->JDWE = *tmpchr[6];
            tmpGprmc->SPEED = atof(tmpchr[7]);
            tmpGprmc->XHQD = *tmpchr[8];
            tmpGprmc->DATE = *tmpchr[9];
            tmpGprmc->MAGE = *tmpchr[10];
            tmpGprmc->ANGLE = *tmpchr[11];
            tmpGprmc->MODE = *tmpchr[12];
            tmpGprmc->SUM = atoi(tmpchr[13]);
            result = true;
        }
    }

    return result;
}
