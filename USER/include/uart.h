#ifndef UART_H
#define UART_H
#include "stm32f37x.h"
#include "BaseDef.h"

#define DRV_BUF_SIZE 300 //串口缓存大小
//#define DATA_BUF_SIZE  	300

/*缓存结构体类型定义*/
typedef struct ucdrv_buf
{
    uint8_t buf[DRV_BUF_SIZE];
    uint16_t wr;
    uint16_t rd;

} UCDRV_BUF;

//串口结构体类型
typedef struct uart_typedef
{
    USART_TypeDef *hard;
    UCDRV_BUF Rsvbuf;
    UCDRV_BUF Sndbuf;
    void (*Init)(void);
    void (*Send)(struct uart_typedef *, uint8_t *, uint8_t);
    uint32_t (*Receive)(struct uart_typedef *, uint8_t *, uint8_t, uint32_t);
} UART_TypeDef;

extern UART_TypeDef UART_GPS, UART_GPRS;

extern void vUart_GPS_Init(void);
extern void vUart_Send(UART_TypeDef *puart, uint8_t *pstring, uint8_t len);
extern uint32_t udwUart_Receive(UART_TypeDef *puart, uint8_t *pstrint, uint8_t len, uint32_t RcvWait1Ms);
extern void vUart_GPRS_Init(void);

extern uint16_t ucDrvBuf_DataLen(const UCDRV_BUF *ucdrv_buf); //驱动缓存中未读取的字节数
extern void vUart_Init(void);                                 //串口初始化
//extern bool 	blDrv_Buf_IsEmpty(const UCDRV_BUF *ucdrv_buf);//判断驱动缓存是否为空
//extern bool 	blDrv_Buf_IsFull(const UCDRV_BUF *ucdrv_buf);//判断驱动缓存是否满
extern uint16_t ucDrvBuf_EmpLen(const UCDRV_BUF *ucdrv_buf);                                //驱动缓存中的剩余空间
extern bool ucDrv_Buf_GetBytes(UCDRV_BUF *ucdrv_buf, uint8_t *ucdata_buf, uint16_t length); //从驱动缓存中读取指定数量的字节到数据缓存数组
extern bool Bufchr(UCDRV_BUF *uc_buf, uint8_t chr, uint16_t ln);                            //从驱动缓存中查找特定字符，读指针
extern bool Buf_cmp(UCDRV_BUF *uc_buf, char *ptchr, uint8_t ln);                            //从驱动缓存中查找特定字符串

#endif
