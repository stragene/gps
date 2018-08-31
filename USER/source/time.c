
#include "stm32f37x.h"
#include "time.h"

uint32_t Ticks;
/********************************************************************
* 功    能：计时器初始化函数
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2018.3.28
**********************************************************************/
void vSysTickInit(void)
{
    //SysTick_Config(SystemCoreClock / 100); //100ms中断一次

    /* set reload register */
    SysTick->LOAD = ((SystemCoreClock / 100) & SysTick_LOAD_RELOAD_Msk) - 1;
    /* set Priority for Systick Interrupt */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    /* Load the SysTick Counter Value */
    SysTick->VAL = 0;
    /* Enable SysTick Timer ,but no interupt */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    //SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    /*关闭SysTick中断*/
    /*NVIC_DisableIRQ(SysTick_IRQn);*/
    /*CTRL寄存器的TICKINIT位和ENABLE位清除*/
    /*SysTick->CTRL &= (!SysTick_CTRL_TICKINT_Msk) & (!SysTick_CTRL_ENABLE_Msk);*/
}

/********************************************************************
* 功    能：计时器清零
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2018.3.28
**********************************************************************/
void vTimerClear(void)
{
}

/********************************************************************
* 功    能：计时器累加
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2018.3.28
**********************************************************************/
void vTimerPlus(void)
{
}

/********************************************************************
* 功    能：计时器暂停
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2018.3.28
**********************************************************************/
void vTimerSuspend(void)
{
}

/********************************************************************
* 功    能：计时器初始化函数
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2016.8.7
**********************************************************************/
/*
void vTimer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruc;
	NVIC_InitTypeDef NVIC_Initstruc;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitStruc.TIM_Period = (1000 - 1); 
	TIM_TimeBaseInitStruc.TIM_Prescaler=8000;//系统频率8M，8000分频，1khz计时频率；
	TIM_TimeBaseInitStruc.TIM_ClockDivision=0;//数字滤波器功能关闭
	TIM_TimeBaseInitStruc.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
//	TIM_TimeBaseInitStruc.TIM_RepetitionCounter 向下自动填装，对TIM15, TIM16 and TIM17有效
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruc);
//	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  //使能定时器中断
	NVIC_Initstruc.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_Initstruc.NVIC_IRQChannelSubPriority=1;
	NVIC_Initstruc.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_Initstruc);
	TIM_Cmd(TIM2,ENABLE);
}
*/
/********************************************************************
* 功    能：计时器中断配置函数
* 输    入：None
* 输    出：None
*           
* 编 写 人：
* 编写日期：2016.8.7
**********************************************************************/
void vIRQ_TIME_Config(void)
{
}
