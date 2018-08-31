#ifndef UART_H
#define UART_H
#include "stm32f37x.h"
#include "BaseDef.h"

#define DRV_BUF_SIZE 300 //���ڻ����С
//#define DATA_BUF_SIZE  	300

/*����ṹ�����Ͷ���*/
typedef struct ucdrv_buf
{
    uint8_t buf[DRV_BUF_SIZE];
    uint16_t wr;
    uint16_t rd;

} UCDRV_BUF;

//���ڽṹ������
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

extern uint16_t ucDrvBuf_DataLen(const UCDRV_BUF *ucdrv_buf); //����������δ��ȡ���ֽ���
extern void vUart_Init(void);                                 //���ڳ�ʼ��
//extern bool 	blDrv_Buf_IsEmpty(const UCDRV_BUF *ucdrv_buf);//�ж����������Ƿ�Ϊ��
//extern bool 	blDrv_Buf_IsFull(const UCDRV_BUF *ucdrv_buf);//�ж����������Ƿ���
extern uint16_t ucDrvBuf_EmpLen(const UCDRV_BUF *ucdrv_buf);                                //���������е�ʣ��ռ�
extern bool ucDrv_Buf_GetBytes(UCDRV_BUF *ucdrv_buf, uint8_t *ucdata_buf, uint16_t length); //�����������ж�ȡָ���������ֽڵ����ݻ�������
extern bool Bufchr(UCDRV_BUF *uc_buf, uint8_t chr, uint16_t ln);                            //�����������в����ض��ַ�����ָ��
extern bool Buf_cmp(UCDRV_BUF *uc_buf, char *ptchr, uint8_t ln);                            //�����������в����ض��ַ���

#endif
