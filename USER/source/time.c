
#include "stm32f37x.h"
#include "time.h"

uint32_t Ticks;
/********************************************************************
* ��    �ܣ���ʱ����ʼ������
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2018.3.28
**********************************************************************/
void vSysTickInit(void)
{
    //SysTick_Config(SystemCoreClock / 100); //100ms�ж�һ��

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

    /*�ر�SysTick�ж�*/
    /*NVIC_DisableIRQ(SysTick_IRQn);*/
    /*CTRL�Ĵ�����TICKINITλ��ENABLEλ���*/
    /*SysTick->CTRL &= (!SysTick_CTRL_TICKINT_Msk) & (!SysTick_CTRL_ENABLE_Msk);*/
}

/********************************************************************
* ��    �ܣ���ʱ������
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2018.3.28
**********************************************************************/
void vTimerClear(void)
{
}

/********************************************************************
* ��    �ܣ���ʱ���ۼ�
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2018.3.28
**********************************************************************/
void vTimerPlus(void)
{
}

/********************************************************************
* ��    �ܣ���ʱ����ͣ
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2018.3.28
**********************************************************************/
void vTimerSuspend(void)
{
}

/********************************************************************
* ��    �ܣ���ʱ����ʼ������
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2016.8.7
**********************************************************************/
/*
void vTimer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruc;
	NVIC_InitTypeDef NVIC_Initstruc;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitStruc.TIM_Period = (1000 - 1); 
	TIM_TimeBaseInitStruc.TIM_Prescaler=8000;//ϵͳƵ��8M��8000��Ƶ��1khz��ʱƵ�ʣ�
	TIM_TimeBaseInitStruc.TIM_ClockDivision=0;//�����˲������ܹر�
	TIM_TimeBaseInitStruc.TIM_CounterMode=TIM_CounterMode_Up;//���ϼ���
//	TIM_TimeBaseInitStruc.TIM_RepetitionCounter �����Զ���װ����TIM15, TIM16 and TIM17��Ч
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruc);
//	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  //ʹ�ܶ�ʱ���ж�
	NVIC_Initstruc.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_Initstruc.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_Initstruc.NVIC_IRQChannelSubPriority=1;
	NVIC_Initstruc.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_Initstruc);
	TIM_Cmd(TIM2,ENABLE);
}
*/
/********************************************************************
* ��    �ܣ���ʱ���ж����ú���
* ��    �룺None
* ��    ����None
*           
* �� д �ˣ�
* ��д���ڣ�2016.8.7
**********************************************************************/
void vIRQ_TIME_Config(void)
{
}
