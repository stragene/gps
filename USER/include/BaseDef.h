/*********************************************************************** 
* Copyright (c) 2009,山东积成电子股份有限公司 All rights reserved.
*
* 文件名称： BaseDef.h
* 描    述： 基本宏定义头文件
* 
* 修改记录：
*
* V1.0，2011.6.10，赵辉  
***********************************************************************/


#ifndef __BASEDEF_H__
#define __BASEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
//#include "integer.h"
    
/* Typedef */
typedef char						  INT8;
typedef short						  INT16;
typedef int							  INT32;
typedef unsigned char				tBoolean;
typedef unsigned char				UINT8;
typedef unsigned short				UINT16;
typedef unsigned int				UINT32;

typedef unsigned char				UCHAR;

//#ifndef BOOL
typedef bool						BOOL;
//#endif 
typedef int							STATUS;
//typedef void			VOID;

typedef float						FLOAT;
typedef double					DOUBLE;

typedef char						CHAR;

typedef unsigned char		U8;

typedef unsigned char		BIT;

typedef unsigned int		U16;

typedef unsigned long		U32;

 

typedef signed char			S8;

typedef signed int			S16;

typedef signed long			S32;

 

typedef union UU16
{

	U16 U16;

	S16 S16;

	U8 U8[2];

	S8 S8[2];

} UU16;

 

typedef union UU32
{

	U32 U32;

	S32 S32;

	UU16 UU16[2];

	U16 U16[2];

	S16 S16[2];

	U8 U8[4];

	S8 S8[4];

} UU32;

/************************* 	
	李先刚添加 
	2012/04/13 
**************************/
typedef enum{
	KeyScanInit,
	KeyScanIdle,
	KeyScanDown,
	KeyScanShort,
	KeyScanLong,
	KeyScanWaitUp	
}KeyScanMode;

typedef enum{
	KeyDownNull,		//按键无效
	KeyDownShort,		//0.2-1S
	KeyDownMid,			//
	KeyDownLong,		//>6s
	KeyDownLock			//
}KeyDownType;

/* Define */
#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

/* BCD码转换为十进制码 */
#define BCD2DEC(x)              ((((x) & 0xF0) >> 4) * 10 + ((x) & 0x0F))

/* 十进制码转换为BCD码 */
#define DEC2BCD(x)              ((((x) / 10) << 4) | ((x) % 10))    /* 0<=x<100 */

/* General Macros */
#define GET_HI_BYTE(w)	(((UINT16)(w) >> 8) & 0x00FF)
#define GET_LO_BYTE(w)	((UINT16)(w) & 0x00FF)
#define GET_HI_WORD(l)	(((l) >> 16) & 0xFFFF)
#define GET_LO_WORD(l)	((l) & 0xFFFF)

#define PUT_HI_BYTE(w,b)	(((UINT16)(b) << 8) | ((w) & 0x00FF))
#define PUT_LO_BYTE(w,b)	((UINT16)(b) | ((w) & 0xFF00))
#define PUT_HI_WORD(l,w)	(((UINT32)(w) << 16) | ((l) & 0x0000FFFF))
#define PUT_LO_WORD(l,w)	((UINT32)(w) | ((l) & 0xFFFF0000))

#define IS_ODD(c)	((((c) % 2) == 1) ? TRUE:FALSE)
#define IS_EVEN(c)	((((c) % 2) == 0) ? TRUE:FALSE)

#define MAX(a,b)	(((a) >= (b)) ? (a) : (b))
#define MIN(a,b)	(((a) <= (b)) ? (a) : (b))

//#define SET_BIT(value,p)		((UINT32)(value) | (p))  /* p = BIT0~31 */
//#define CLEAR_BIT(value,p)		((UINT32)(value) & ~(p))
#define REVERSE_BIT(value,p)	((UINT32)(value) ^ (p))

#define SET_WORD_BIT(value,p)		((UINT16)(value) | (p))  /* p = BIT0~15 */
#define CLEAR_WORD_BIT(value,p)		((UINT16)(value) & ~(p))
#define REVERSE_WORD_BIT(value,p)	((UINT16)(value) ^ (p))

#define TEST_BIT(value,p)			((UINT32)(value) & (p))   /* p = BIT0~31 */

#define GET_BIT(value,p)			(((UINT32)(value) >> (p)) & 1)    /* p = 0~31 获取第p位*/
#define GET_BITS(value,p,n)			(((UINT32)(value) >> (p)) & ~(~0 << n))	//从第p位开始共获取n位

#define MAKE_WORD(hbyte,lbyte)		(((UINT8)(lbyte)) | (((UINT8)(hbyte))<<8))
#define MAKE_LONG(hword,lword)		(((UINT16)(lword)) | (((UINT16)(hword))<<16))

#define READ_BYTE(address)			*((volatile UINT8  *) address)
#define READ_WORD(address)			*((volatile UINT16 *) address)
#define READ_LONG(address)			*((volatile UINT32 *) address)

#define BCD_TO_BIN(bcd)		( ((((bcd)&0xF0)>>4)*10) + ((bcd)&0x0F) )
#define BIN_TO_BCD(bin)		( (((bin)/10)<<4) + ((bin)%10) )

#define MSB(x)	(((x) >> 8) & 0xff)	  /* most signif byte of 2-byte integer */
#define LSB(x)	((x) & 0xff)		  /* least signif byte of 2-byte integer*/
#define MSW(x) (((x) >> 16) & 0xffff) /* most signif word of 2-word integer */
#define LSW(x) ((x) & 0xffff) 		  /* least signif byte of 2-word integer*/

/* swap the MSW with the LSW of a 32 bit integer */
#define WORDSWAP(x) (MSW(x) | (LSW(x) << 16))

#define LLSB(x)	((x) & 0xff)		/* 32bit word byte/word swap macros */
#define LNLSB(x) (((x) >> 8) & 0xff)
#define LNMSB(x) (((x) >> 16) & 0xff)
#define LMSB(x)	 (((x) >> 24) & 0xff)
#define LONGSWAP(x) ((LLSB(x) << 24) | \
	(LNLSB(x) << 16)| \
	(LNMSB(x) << 8) | \
	(LMSB(x)))

#define FOREVER	for (;;)

#define max(x, y)	(((x) < (y)) ? (y) : (x))
#define min(x, y)	(((x) < (y)) ? (x) : (y))

/* return status values */
#ifndef WAIT
#define WAIT 1
#endif

#ifndef OK_1
#define OK_1 0
#endif

#ifndef ERROR_1
#define ERROR_1 -1
#endif

#ifndef DENY
#define DENY -2
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define bzero(a, b)		memset(a, 0, b)
#define bcopy(s, d, c)	memmove(d, s, c)

#define LEAP_YEAR(y)	((((y)%4==0) && ((y)%100!=0))||((y)%400==0))	//true闰年 false普通年

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BASEDEF_H__ */

