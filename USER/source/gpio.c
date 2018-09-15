
#include "gpio.h"
#include "stm32f37x.h"


void vRunLed_Init(void)
{
    GPIO_InitTypeDef GPIO_Initstruc;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_Initstruc.GPIO_Pin = GPIO_Pin_0;
    GPIO_Initstruc.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Initstruc.GPIO_OType = GPIO_OType_PP;
    GPIO_Initstruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Initstruc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_Initstruc);
}
