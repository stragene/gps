#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "sim800.h"
#include "gps.h"
#include "onenet.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

void vBoardInit(void);
void vSim800_TCPInit(void *pGprs);
void vRunLed_Init(void);
void vTaskRunLed(void);
//void vTaskPPPRead(void);
void vTaskPublish(void);
SemaphoreHandle_t xSemGprsRsvd;
/*QueueHandle_t handQueueU1Frame;*/

void vBoardInit(void)
{
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    vInnerFlash_Init();
    vRunLed_Init();
    //Sim800GPRSInit();
    pSim800GPS->Init();
}

void vSim800_TCPInit(void *pGprsDev)
{
    struct gprs_dev *pGprs = pGprsDev;
    pGprs->PowerEn();
    pGprs->delay(TICKS_1S);
    pGprs->OnOff();
    pGprs->delay(TICKS_1S);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    while (1)
    {
        if (pGprs->SendCmd("AT\r\n", "OK", TICKS_1S, 6) &&
            pGprs->SendCmd("ATE0\r\n", "OK", TICKS_500MS, 6) &&
            pGprs->SendCmd("AT+CPIN?\r\n", "READY", TICKS_1S, 3) &&
            pGprs->SendCmd("AT+CSQ\r\n", "", TICKS_500MS, 3) &&
            pGprs->SendCmd("AT+CGATT?\r\n", "", TICKS_500MS, 3) &&
            pGprs->SendCmd("AT+CIPSHUT\r\n", "SHUT OK", 2 * TICKS_1S, 3) &&
            pGprs->SendCmd("AT+CSTT=\"CMNET\"\r\n", "OK", 2 * TICKS_1S, 3) &&
            pGprs->SendCmd("AT+CIICR\r\n", "OK", 90 * TICKS_1S, 3) &&
            pGprs->SendCmd("AT+CIFSR\r\n", "", 2 * TICKS_1S, 3) &&
            //pGprs->SendCmd("AT+CIPSTART=\"TCP\",\"116.247.119.165\",9336\r\n", "CONNECT", 2000, 3))
            pGprs->SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.39\",6002\r\n", "CONNECT", 2 * TICKS_1S, 3))
        //pGprs->SendCmd("AT+CIPSTART=\"TCP\",\"124.128.119.120\",6002\r\n", "CONNECT", 2000, 3))
        {
            break;
        }
    }
}

int main(void)
{
    vBoardInit();
    vSim800GPRSInit(pSim800GPRS);
    vSim800_TCPInit(pSim800GPRS);
    onenetConnect();

    //vSemaphoreCreateBinary(xSemGprsRsvd);
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));
    //xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate((void *)vTaskPublish, "Publish", 4 * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

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
void vTaskPublish(void)
{
    float lon = 116.9905972;
    for (;;)
    {
        lon = lon + 0.0001;
        //onenetSendData(116.9905972, 36.6484770);
        onenetSendData(lon, 36.6484770);
        //onenetPublish(1111, 2222);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}