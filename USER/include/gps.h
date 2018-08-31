#ifndef GPS_H
#define GPS_H

#include "stm32f37x.h"
#include "BaseDef.h"
#include "uart.h"

typedef struct gps_gprmc
{
	bool 			IsValid;	//帧合法判断
	char 		 	Command;	//命令
	char 			UTC;		//时间
	char 			VA;		//数据有效标记
	double 			WD;		//纬度值
	char 			WDNS;		//纬度区域
	double 			JD;		//经度值
	char 			JDWE;		//经度区域
	double 			SPEED;	 	//速度
	double 			XHQD;		//相对方向
	char 			DATE;		//日期
	float 			MAGE;		//磁偏角
	float 			ANGLE;		//磁偏角方向
	char 			MODE;		//模式
	uint32_t 		SUM;		//校验和
}GpsGprmc_TypeDef;


extern void 		vGps_Init(void);
extern void 		vGps_EnDen();
extern bool 		Gps_ReadFromBuf(GpsGprmc_TypeDef *tmpGprmc);
	



/*
typedef struct gps_time
{
	long int year;
    	long int month;
    	long int day;
    	long int hour;
    	long int minute;
    	long int second;	
}GpsTime_TypeDef;
typedef struct gps_gpgga
{
	unsigned char Command[7];
	unsigned char UTC[7];
	unsigned char WD[10];
	unsigned char WDNS[2];
	unsigned char JD[11];
	unsigned char JDWE[2];
	unsigned char POS_STATUS[2];
	unsigned char WXS[3];
	unsigned char HDOP[4];
	unsigned char WH[6];
	unsigned char DA1[2];
	unsigned char DH[5];
	unsigned char DA2[4];
	unsigned char SUM[3];
}GpsGpgga_TypeDef;

typedef struct gps_inf
{
	unsigned char 		VA;
	unsigned char 		WDNS;
	double 			WD;
	unsigned char 		JDWE;
	double 			JD;
	unsigned char 		POS_STATUS;
	unsigned char 		WXS;
	float 			HDOP;
	float 			SPEED;
	float 			XHQD;
	GpsTime_TypeDef 	TIME;
}GPS_INF;

*/
#endif

