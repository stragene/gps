#include "stm32f37x.h"
#include "gps.h"
#include <stdlib.h>
#include <string.h>



/* �ṹ�嶨��-- ------------------------------------------------------------------*/



/*******************************************************************************
* ��������           : vGps_Init()
* ��������           : GPS�����ʼ��
* ��    ��           : 
* ��    ��           : 
* �� �� ֵ           : 1
* ��    ע           :                       
*******************************************************************************/
void vGps_Init(void)
{	
	/*PA2����ΪGPSʹ��*/
	GPIO_InitTypeDef GPIO_Initstruc;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_Initstruc.GPIO_Pin  =GPIO_Pin_2;
	GPIO_Initstruc.GPIO_Mode =GPIO_Mode_OUT;
	GPIO_Initstruc.GPIO_OType=GPIO_OType_PP;//��©
	GPIO_Initstruc.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Initstruc.GPIO_PuPd =GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_Initstruc);
	/*B6��B7����Ϊ����1,GPS*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_7);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_7);
	GPIO_Initstruc.GPIO_Mode=GPIO_Mode_AF;
	GPIO_Initstruc.GPIO_OType=GPIO_OType_PP;
	GPIO_Initstruc.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Initstruc.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Initstruc.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_Initstruc);
	/*���ڳ�ʼ��*/
	UART_GPS.Init();
}

/*******************************************************************************
* ��������           : vGps_EnDen()
* ��������           : GPSʹ��/����
* ��    ��           : 
* ��    ��           : 
* �� �� ֵ           : 1
* ��    ע           :                       
*******************************************************************************/
void vGps_PEn(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);
}
/*******************************************************************************
* ��������           : vGps_EnDen()
* ��������           : GPSʹ��/����
* ��    ��           : 
* ��    ��           : 
* �� �� ֵ           : 1
* ��    ע           :                       
*******************************************************************************/
void vGps_EnDen(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET);
}

/*******************************************************************************
* ��������           : GPS_Analysis()
* ��������           : ��GPS����֡���н�����װ�뵽�ṹ����
* ��    ��           : 
* ��    ��           : 
* �� �� ֵ           : 1
* ��    ע           :                       
*******************************************************************************/
bool Gps_ReadFromeBuf(GpsGprmc_TypeDef *tmpGprmc) 
{
	char     		ucGPSFrame[DRV_BUF_SIZE];
	char			*pGPSFrame;
	uint32_t 		i= 0;
	uint32_t 		j= 0;
	char 			*tmpchr[14];
	bool 			result=false;

	/*�����������ȡȫ��֡*/
	for(i=0; i< DRV_BUF_SIZE; i++)
	{
		ucGPSFrame[i]= UART_GPS.Rsvbuf.buf [ UART_GPS.Rsvbuf.rd];
		UART_GPS.Rsvbuf.rd= ( UART_GPS.Rsvbuf.rd+ DRV_BUF_SIZE )% DRV_BUF_SIZE;
	}
	/*�ҵ�$GPRMC*/
	do
	{
		pGPSFrame= strchr(ucGPSFrame, '$');
	}
	while  ( strncmp(pGPSFrame, "$GPRMC", 6) != 0);
	
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

	/*�Ӵ�����ת��*/
	tmpchr[0]= strtok( pGPSFrame, ","); 	
	while (tmpchr[j] != NULL && j< 12)				//�Դ˴洢��1~12��֡����12֡��*��β
	{
		tmpchr[++j]= strtok( NULL, ",");	
	}

	pGPSFrame  = tmpchr[12];						// mode*sum
	tmpchr[12] = strtok(pGPSFrame, "*");			// mode
	tmpchr[13] = strtok(NULL, "*");					// sum
	if (j != 12)
	{
		result= false;
	}
	else
	{
		if( !strncmp( tmpchr[12], "V", 1) )
		{
			result= false;
		}

		else
		{
			tmpGprmc->UTC 		= *tmpchr[1];
			tmpGprmc->VA		= *tmpchr[2];
			tmpGprmc->WD 		= atof(tmpchr[3]);
			tmpGprmc->WDNS 		= *tmpchr[4];
			tmpGprmc->JD		= atof(tmpchr[5]);
			tmpGprmc->JDWE 		= *tmpchr[6];
			tmpGprmc->SPEED 	= atof(tmpchr[7]);
			tmpGprmc->XHQD 		= *tmpchr[8];
			tmpGprmc->DATE 		= *tmpchr[9];
			tmpGprmc->MAGE 		= *tmpchr[10];
			tmpGprmc->ANGLE 	= *tmpchr[11];
			tmpGprmc->MODE 		= *tmpchr[12];
			tmpGprmc->SUM 		= atoi(tmpchr[13]);
			result 				= true;
		}
	}

	return result;
}

