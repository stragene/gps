#ifndef SIM800_H
#define SIM800_H
#include "stm32f37x.h"

extern char cSim800_Rsv;
extern char *pcSim800Rsv;

extern void vSim800_HardInit(void);
extern void vSim800_pEn(void);
extern void vSim800_pDen(void);
extern void vSim800_OnOff(void);
extern void vSim800_Reset(void);
extern void vSim800_SndCmd(char *cmd);
extern uint32_t Sim800_RsvCmd(char *cmd, uint32_t len, uint32_t delayMs);

#endif
