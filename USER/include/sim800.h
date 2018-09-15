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
    bool (*SendCmd)(char *pcmd, char* response, uint32_t timeout, uint32_t retry);
    void (*delay)(uint32_t);
};

extern struct gprs_dev *pSim800GPRS;
#endif
