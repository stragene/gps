#ifndef FLASH_H
#define FLASH_H

#include "stm32f37x.h"


extern void Flash_Write(uint32_t addr,uint8_t ByteData);//дflash
extern uint8_t Flash_Read(uint32_t addr);//��flash
extern void vInnerFlash_Init(void);



#endif
