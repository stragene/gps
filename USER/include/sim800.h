#ifndef SIM800_H
#define SIM800_H
#include "stm32f37x.h"
#include "uart.h"

struct gprs_dev 
{
    void (*Init)(void);
    void (*OnOff)(void);
    void (*PowerEn)(void);
    void (*PowerDen)(void);
    void (*Write)(UartDef *port, uint32_t *data, uint8_t datalen);
    uint32_t (*Read)(UartDef *port, uint8_t *dest, uint32_t len, uint32_t delay);
    void (*delay)(uint32_t);
};

extern struct gprs_dev *pSim800GPRS;
#endif
