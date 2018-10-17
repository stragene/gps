#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "sim800.h"
#include "gps.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
//#include "netif/ppp/pppos.h"
//#include "netif/ppp/pppapi.h"
//#include "lwip/tcpip.h"

void vBoardInit(void);
void vSim800_TCPInit(void);
void vRunLed_Init(void);
void vTaskRunLed(void);
void vTaskPPPRead(void);
SemaphoreHandle_t xSemGprsRsvd;
/*QueueHandle_t handQueueU1Frame;*/

void vBoardInit(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    vInnerFlash_Init();
    vRunLed_Init();
    //pSim800GPRS->Init();
    pSim800GPS->Init();
}

void vSim800_TCPInit(void)
{
    pSim800GPRS->Init();
    pSim800GPRS->PowerEn();
    pSim800GPRS->delay(1000);
    pSim800GPRS->OnOff();
    pSim800GPRS->delay(2000);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    while (1)
    {
        if (pSim800GPRS->SendCmd("AT\r\n", "OK", 1000, 6) &&
            pSim800GPRS->SendCmd("ATE0\r\n", "OK", 1000, 6) &&
            pSim800GPRS->SendCmd("AT+CPIN?\r\n", "READY", 500, 3) &&
            pSim800GPRS->SendCmd("AT+CSQ\r\n", "", 500, 3) &&
            pSim800GPRS->SendCmd("AT+CGATT?\r\n", "", 500, 3) &&
            pSim800GPRS->SendCmd("AT+CIPSHUT\r\n", "SHUT OK", 2000, 3) &&
            pSim800GPRS->SendCmd("AT+CSTT=\"CMNET\"\r\n", "OK", 2000, 3) &&
            pSim800GPRS->SendCmd("AT+CIICR\r\n", "OK", 90000, 3) &&
            pSim800GPRS->SendCmd("AT+CIFSR\r\n", "", 2000, 3)) && 
            pSim800GPRS->SendCmd("AT+CIPSTART=\"TCP\",\"116.247.119.165\",9336\r\n", "CONNECT", 2000, 3))
            {
                break;
            }
    }
}

int main(void)
{
    vBoardInit();
    vSim800_TCPInit();
    vSemaphoreCreateBinary(xSemGprsRsvd);
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));
    xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    //xTaskCreate((void *)vTaskComInit, "ComInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
    while (1)
        ;
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
