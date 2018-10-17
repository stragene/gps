#ifndef SIM800_H
#define SIM800_H
#include "stm32f37x.h"
#include "uart.h"

struct gprs_dev
{
    UartDef *Interface;
    void (*Init)(void);
    void (*OnOff)(void);
    void (*PowerEn)(void);
    void (*PowerDen)(void);
    bool (*SendCmd)(char *pcmd, char *response, uint32_t timeout, uint32_t retry);
    uint32_t (*SendData)(uint8_t *pbuf, uint32_t len);
    void (*AutoReadEn)(void);
    void (*delay)(uint32_t);
};

extern struct gprs_dev *pSim800GPRS;
#endif
