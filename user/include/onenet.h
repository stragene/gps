#ifndef _ONENET_H
#define _ONENET_H
#include "stm32f37x.h"
#include "utils.h"
#include "mqtt.h"

#define PROD_ID "173278" //修改为自己的产品ID
//#define SN "201608160002"           //修改为自己的设备唯一序列号
#define REG_CODE "6TM7OkhNsTjATvFx" //修改为自己的产品注册码
#define API_ADDR "api.heclouds.com"
#define API_KEY "reC6LwIXkj6j9=AV5CCo8g6qijg="
#define DEVICE_NAME "Sim800"
#define REG_PKT_HEAD "POST http://" API_ADDR "/devices HTTP/1.1\r\n" \
                     "api-key: reC6LwIXkj6j9=AV5CCo8g6qijg="         \
                     "Host: " API_ADDR "\r\n"
#define REG_PKT_BODY "{\"title\":\"" DEVICE_NAME "\"}"
#define STRLEN 64

/* mqtt连接参数 */
#define KEEP_ALIVE 120
#define CLEAN_SESSION 0
struct MqttSampleContext
{
    //    int epfd;
    //    int mqttfd;
    uint32_t sendedbytes;
    struct MqttContext mqttctx[1];
    struct MqttBuffer mqttbuf[1];

    char *host;
    unsigned short port;

    char *proid;
    char *devid;
    char *apikey;

    int dup;
    enum MqttQosLevel qos;
    int retain;

    uint16_t pkt_to_ack;
    char cmdid[70];
};
extern int onenetGetDevID(struct MqttSampleContext *ctx);
extern int onenetConnect(void);
extern int onenetSubscribe(char **topic, int num);
extern int onenetPublish(int temp, int humi);
extern int onenetResCmd(char *resp);
extern int onenetSendData(int temp, int humi);
#endif