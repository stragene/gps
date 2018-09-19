#ifndef UART_H
#define UART_H
#include "stm32f37x.h"
#include <stdbool.h>

/*串口驱动缓存*/
#define BUF_SIZE 0xFF

struct ucbuf
{
    uint8_t data[BUF_SIZE];
    uint32_t rd;
    uint32_t wr;
};

typedef struct uartdef
{
    USART_TypeDef *handler;
    struct ucbuf *pRsvbuf;
    struct ucbuf *pSndbuf;
} UartDef;

extern UartDef *pUartGPRS, *pUartGPS;

extern void vUart_GPS_Init(void);
extern void vUart_GPRS_Init(void);

extern uint32_t Uart_Read(UartDef *puart, uint8_t *buf, uint32_t count);
extern uint32_t Uart_IdleRead(UartDef *puart, uint8_t *buf, uint32_t count, uint32_t idleMs);
extern uint32_t Uart_OnceRead(UartDef *puart, uint8_t *buf, uint32_t count, uint32_t delay);

extern int32_t Uart_OnceWrite(UartDef *puart, const uint8_t *buf, uint32_t count, uint32_t delay);
extern void Uart_Write(UartDef *puart, const uint8_t *pbuf, uint32_t count);

extern void Uart_ITReadEnable(UartDef *puart);
extern void Uart_ITReadDisable(UartDef *puart);

extern uint32_t uwBuf_EmpLen(const struct ucbuf *buf);
extern uint32_t uwBuf_UnReadLen(const struct ucbuf *buf);
extern bool blBufchr(struct ucbuf *uc_buf, uint8_t chr, uint16_t ln);
extern bool blBufcmp(struct ucbuf *uc_buf, char *ptchr, uint8_t ln);
extern void vBuf_Clear(struct ucbuf *buf);

#endif