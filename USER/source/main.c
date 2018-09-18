#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "sim800.h"
#include "gps.h"

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"

void vBoardInit(void);
void vRunLed_Init(void);
void vTaskComInit(void);
void vTaskRunLed(void);

SemaphoreHandle_t xSemaphore_MeterReceived;
QueueHandle_t handQueueU1Frame;

void vSim800_TestInit(void)
{
    pSim800GPRS->Init();
    pSim800GPRS->PowerEn();
    pSim800GPRS->delay(1000);
    pSim800GPRS->OnOff();
    pSim800GPRS->delay(2000);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#if 1
    while (1)
    {
        pSim800GPRS->SendCmd("AT\r\n", "AT\r\r\nOK\r\n", 1000, 6);
        pSim800GPRS->SendCmd("ATE0\r\n", "ATE0\r\r\nOK\r\n", 1000, 6);
        pSim800GPRS->SendCmd("AT+CPIN?\r\n", "\r\nAT+CPIN: READY", 500, 3);
        pSim800GPRS->SendCmd("AT+CSQ\r\n", "", 500, 3);
        pSim800GPRS->SendCmd("AT+CGATT?\r\n", "", 500, 3);
        pSim800GPRS->SendCmd("AT+CIPSHUT\r\n", "\r\nSHUT OK\r\n", 2000, 3);
    }
#endif
}

int main(void)
{
    vBoardInit();
    vSim800_TestInit();

    //vSemaphoreCreateBinary(xSemaphore_MeterReceived);
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));
    xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    //xTaskCreate((void *)vTaskComInit, "ComInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
}

void vBoardInit(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    vInnerFlash_Init();
    vRunLed_Init();
    pSim800GPRS->Init();
    pSim800GPS->Init();
}

void vTaskRunLed(void)
{
    for (;;)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
