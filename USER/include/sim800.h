#ifndef SIM800_H
#define SIM800_H
#include "stm32f37x.h"
#include "uart.h"
#include "FreeRTOS.h"

#define TICKS_1S configTICK_RATE_HZ
#define TICKS_500MS (configTICK_RATE_HZ / 2)
#define TICKS_50MS (configTICK_RATE_HZ / 20)

struct gprs_dev
{
    UartDef *Interface;
    //void (*Init)(void);
    void (*OnOff)(void);
    void (*PowerEn)(void);
    void (*PowerDen)(void);
    bool (*SendCmd)(char *pcmd, char *response, uint32_t timeout, uint32_t retry);
    uint32_t (*SendData)(uint8_t *pbuf, uint32_t len, uint32_t timeout);
    void (*AutoReadEn)(void);
    uint32_t (*Read)(uint8_t *buf, uint32_t count);
    void (*delay)(uint32_t);
};

extern struct gprs_dev *pSim800GPRS;
extern void vSim800GPRSInit(struct gprs_dev *pGPRS);
#endif
