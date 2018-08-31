#ifndef SIM800_H
#define SIM800_H
#include "stm32f37x.h"
#include "uart.h"

extern char cSim800_Rsv;
extern char *pcSim800Rsv;

#define vSim800_pEn() GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET)
#define vSim800_PDen() GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);

extern void vSim800_HardInit(void);
extern void vSim800_SndCmd(UART_TypeDef *puart, char *cmd);
extern uint32_t Sim800_RsvCmd(UART_TypeDef *puart, char *cmd, uint32_t len, uint32_t delayMs);

#endif
