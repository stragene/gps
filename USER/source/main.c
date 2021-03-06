#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"
#include "sim800.h"
#include "gps.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "netif/ppp/pppapi.h"
#include "lwip/tcpip.h"

void vBoardInit(void);
void vSim800_PPPInit(void);
void vRunLed_Init(void);
void vTaskRunLed(void);
void vTaskPPPRead(void);
void vTaskTcpIpInit(void);
//SemaphoreHandle_t xSemGprsRsvd;
static void status_cb(ppp_pcb *pcb, int err_code, void *ctx);
static u32_t output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx);
ppp_pcb *pppGprs;
struct netif pppGprs_netif;
uint8_t databuf[BUF_SIZE];
uint8_t *pdatabuf = databuf;

void vSim800_PPPInit(void)
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
            /*pSim800GPRS->SendCmd("AT+CIPSHUT\r\n", "SHUT OK", 2000, 3) &&*/
            pSim800GPRS->SendCmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n", "OK", 1000, 3)) // &&
            /*pSim800GPRS->SendCmd("ATD*99#\r\n", "CONNECT", 2000, 3) &&*/
            /*pSim800GPRS->SendCmd("AT+CIFSR\r\n", "", 2000, 3))*/
            break;
        ;
    }
}

int main(void)
{
    vBoardInit();
    vSim800_PPPInit();
    //vSemaphoreCreateBinary(xSemGprsRsvd);
    xTaskCreate((void *)vTaskTcpIpInit, "TcpipInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    //xTaskCreate((void *)vTaskRunLed, "RunLed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
    //handQueueU1Frame =xQueueCreate(2, sizeof(uint8_t));

    //xTaskCreate((void *)vTaskComInit, "ComInit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    while (1)
        ;
}

void vBoardInit(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    vInnerFlash_Init();
    vRunLed_Init();
    //pSim800GPRS->Init();
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

void vTaskPPPRead(void)
{
    volatile uint32_t readlen = 0;
    volatile uint32_t datalen = 0;
    uint8_t count7e = 0;
    USART_ITConfig(pUartGPRS->handler, USART_IT_RXNE, ENABLE);
    for (;;)
    {
        readlen = uwBuf_UnReadLen(pUartGPRS->pRsvbuf);
        if (readlen == 0)
            continue;
        else
        {
            *pdatabuf = pUartGPRS->pRsvbuf->data[pUartGPRS->pRsvbuf->rd];
            pUartGPRS->pRsvbuf->rd = (++pUartGPRS->pRsvbuf->rd) % BUF_SIZE;
            if (*pdatabuf == 0x7e)
            {
                count7e++;
            }
            /*had 1 0x7e*/
            if (count7e & 0x1)
            {
                pdatabuf++;
                datalen++;
            }
            /* no 0x7e or had 2 0x7e*/
            else
            {
                pdatabuf = &databuf[0];
                if (datalen > 0)
                {
                    pppos_input_tcpip(pppGprs, pdatabuf, readlen);
                    datalen = 0;
                }
            }
        }
    }
}
/*************************************************
*创建PPPGPRS接收任务,传递给内核:pppos_input_tcpip();
*发送ATD*99
*tcpip_init(NULL,NULL);
*pppos_create()
*ppp_connect()
*ppp_set_default()
*************************************************/
void vTaskTcpIpInit(void)
{
    uint8_t ctx = 0;
    uint16_t holdoff;
    xTaskCreate((void *)(vTaskPPPRead), "ppp_read", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    Uart_OnceWrite(pUartGPRS, (uint8_t *)"ATD*99***1#\r\n", 13, 100);
    tcpip_init(NULL, NULL);
    pppGprs = pppos_create(&pppGprs_netif, output_cb, status_cb, &ctx);
    //vTaskDelay(10 / portTICK_RATE_MS);
    vTaskDelay(10 / portTICK_RATE_MS);
    ppp_set_auth(pppGprs, PPPAUTHTYPE_ANY, "card", "card");
    if (ERR_OK != ppp_connect(pppGprs, holdoff))
    {
        while (1)
            ;
    }
    ppp_set_default(pppGprs);
    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
static void status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
    struct netif *pppif = ppp_netif(pcb);
    LWIP_UNUSED_ARG(ctx);
    switch (err_code)
    {
    case PPPERR_NONE:
    {
        return;
#if LWIP_DNS
        const ip_addr_t *ns;
#endif  /* LWIP_DNS */
        /*printf("status_cb: Connected\n");*/
#if PPP_IPV4_SUPPORT
/*
*      printf("   our_ipaddr  = %s\n", ipaddr_ntoa(&pppif->ip_addr));
*      printf("   his_ipaddr  = %s\n", ipaddr_ntoa(&pppif->gw));
*      printf("   netmask     = %s\n", ipaddr_ntoa(&pppif->netmask));*/
#if LWIP_DNS
        ns = dns_getserver(0);
        /*printf("   dns1        = %s\n", ipaddr_ntoa(ns));*/
        ns = dns_getserver(1);
        /*printf("   dns2        = %s\n", ipaddr_ntoa(ns));*/
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
        /*printf("   our6_ipaddr = %s\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));*/
#endif /* PPP_IPV6_SUPPORT */
        break;
    }
    case PPPERR_PARAM:
    {
        //printf("status_cb: Invalid parameter\n");
        break;
    }
    case PPPERR_OPEN:
    {
        //printf("status_cb: Unable to open PPP session\n");
        break;
    }
    case PPPERR_DEVICE:
    {
        //printf("status_cb: Invalid I/O device for PPP\n");
        break;
    }
    case PPPERR_ALLOC:
    {
        //printf("status_cb: Unable to allocate resources\n");
        break;
    }
    case PPPERR_USER:
    {
        //printf("status_cb: User interrupt\n");
        break;
    }
    case PPPERR_CONNECT:
    {
        //printf("status_cb: Connection lost\n");
        break;
    }
    case PPPERR_AUTHFAIL:
    {
        //printf("status_cb: Failed authentication challenge\n");
        break;
    }
    case PPPERR_PROTOCOL:
    {
        //printf("status_cb: Failed to meet protocol\n");
        break;
    }
    case PPPERR_PEERDEAD:
    {
        //printf("status_cb: Connection timeout\n");
        break;
    }
    case PPPERR_IDLETIMEOUT:
    {
        //printf("status_cb: Idle Timeout\n");
        break;
    }
    case PPPERR_CONNECTTIME:
    {
        //printf("status_cb: Max connect time reached\n");
        break;
    }
    case PPPERR_LOOPBACK:
    {
        //printf("status_cb: Loopback detected\n");
        break;
    }
    default:
    {
        //printf("status_cb: Unknown error code %d\n", err_code);
        break;
    }
    }
    /* ppp_close() was previously called, don't reconnect */
    if (err_code == PPPERR_USER)
    {
        /* ppp_free(); -- can be called here */
        return;
    }
    /*
   * Try to reconnect in 30 seconds, if you need a modem chatscript you have
   * to do a much better signaling here ;-)
   */
    ppp_connect(pcb, 30);
    /* OR ppp_listen(pcb); */
}
static u32_t output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
    //return Uart_OnceWrite(UART, data, len);
    return pSim800GPRS->Send(data, len);
}