#include "uart.h"
#include "onenet.h"
#include "sim800.h"

char g_cmdid[STRLEN];
static char *topics[] = {"gps", "test2"};
static struct MqttSampleContext onenetContex;
static struct MqttSampleContext *onenetCtx = &onenetContex;
void sendHttpPkt(char *phead, char *pbody)
{
    char sendBuf0[20];
    char sendBuf1[500];

    sprintf(sendBuf1, "%s%d\r\n\r\n%s", phead, strlen(pbody), pbody);
    printf("send HTTP pkt:\r\n%s\r\n", sendBuf1);
    pSim800GPRS->SendCmd("AT+CIPSEND=%d\r\n", ">", 500, 1);
    pSim800GPRS->AutoReadEn();
    pSim800GPRS->SendData((uint8_t *)sendBuf1, strlen(sendBuf1));
}

/**
  * @brief     串口数据解析
  * @param     buffer，串口接收数据缓存
  * @param     plen， 用于保存解析后EDP数据的长度
  * @attention 由于串口数据参杂着8266模块的上报信息，需要将串口数据进行解析
  *            根据模块手册，若收到 +IPD, 则表明收到了服务器的下发数据
  * @retval    串口数据中EDP数据的首指针，若没有数据则返回NULL
  */
char *uartDataParse(char *buffer, int32_t *plen)
{
    char *p;
    char *pnum;
    int32_t len;
    if ((p = strstr(buffer, "CLOSED")) != NULL)
    {
        printf("tcp connection closed\r\n");
    }
    if ((p = strstr(buffer, "WIFI DISCONNECT")) != NULL)
    {
        printf("wifi disconnected\r\n");
    }
    if ((p = strstr(buffer, "WIFI CONNECTED")) != NULL)
    {
        printf("wifi connected\r\n");
    }
    if ((p = strstr(buffer, "+IPD")) != NULL)
    {
        pnum = p + 5;       //跳过头部的 "+IPD,"，指向长度字段的首地址
        p = strstr(p, ":"); //指向长度字段末尾
        *(p++) = '\0';      //长度字段末尾添加结束符，p指向服务器下发的第一个字节
        len = atoi(pnum);
        //printf("rcv %d data from OneNET:\r\n", len);
        //hexdump(p, len);    //打印接收数据
        *plen = len;
        return p;
    }
    return NULL;
}

/**
  * @brief  非透传模式发送MQTT报文
  */
void sendPkt(char *p, int len)
{
    char sendBuf[30] = {0};

    /* 非透传模式先发送AT+CIPSEND=X */
    //sprintf(sendBuf, "AT+CIPSEND=%d\r\n", len);
    //SendCmd(sendBuf, ">", 500);
    pSim800GPRS->SendCmd("AT+CIPSEND=%d\r\n", ">", 500, 1);
    /* MQTT设备连接包，发送 */
    pSim800GPRS->SendData(p, len);
}

//-------------------------------- Commands ------------------------------------------------------
static int MqttSample_Connect(struct MqttSampleContext *ctx, char *proid, char *auth_info, const char *devid, int keep_alive, int clean_session)
{
    int err, flags;
    printf("product id: %s\r\nsn: %s\r\ndeviceid: %s\r\nkeepalive: %d\r\ncleansession: %d\r\nQoS: %d\r\n",
           proid, auth_info, devid, keep_alive, clean_session, MQTT_QOS_LEVEL0);
    err = Mqtt_PackConnectPkt(ctx->mqttbuf, keep_alive, devid,
                              clean_session, NULL,
                              NULL, 0,
                              MQTT_QOS_LEVEL0, 0, proid,
                              auth_info, strlen(auth_info));
    if (MQTTERR_NOERROR != err)
    {
        printf("Failed to pack the MQTT CONNECT PACKET, errcode is %d.\n", err);
        return -1;
    }

    return 0;
}

static int MqttSample_RecvPkt(void *arg, void *buf, uint32_t count)
{
    int i;
    int rcv_len = 0, onenetdata_len = 0;
    uint8_t buffer[128] = {0};
    char *p = NULL;
    if (uwBuf_UnReadLen(pUartGPRS->pRsvbuf))
    {
        onenetdata_len = Uart_Read(pUartGPRS, buf, count);
    }
    return onenetdata_len;
}

static int MqttSample_SendPkt(void *arg, const struct iovec *iov, int iovcnt)
{
    char sendbuf[1024];
    int len = 0;
    int bytes;
    int i = 0, j = 0;
    printf("send one pkt\n");
    for (i = 0; i < iovcnt; ++i)
    {
        char *pkg = (char *)iov[i].iov_base;
        for (j = 0; j < iov[i].iov_len; ++j)
        {
            printf("%02X ", pkg[j] & 0xFF);
        }
        printf("\n");

        memcpy(sendbuf + len, iov[i].iov_base, iov[i].iov_len);
        len += iov[i].iov_len;
    }

    sendPkt(sendbuf, len);
    printf("send over\n");
    return bytes;
}

//------------------------------- packet handlers -------------------------------------------
static int MqttSample_HandleConnAck(void *arg, char flags, char ret_code)
{
    printf("Success to connect to the server, flags(%0x), code(%d).\n",
           flags, ret_code);
    return 0;
}

static int MqttSample_HandlePingResp(void *arg)
{
    printf("Recv the ping response.\n");
    return 0;
}

static int MqttSample_HandlePublish(void *arg, uint16_t pkt_id, const char *topic,
                                    const char *payload, uint32_t payloadsize,
                                    int dup, enum MqttQosLevel qos)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext *)arg;
    ctx->pkt_to_ack = pkt_id;
    ctx->dup = dup;
    ctx->qos = qos;
    printf("dup: %d, qos: %d, id: %d\r\ntopic: %s\r\npayloadsize: %d  payload: %s\r\n",
           dup, qos, pkt_id, topic, payloadsize, payload);

    /*fix me : add response ?*/

    //get cmdid
    //$creq/topic_name/cmdid
    memset(g_cmdid, STRLEN, 0);
    if ('$' == topic[0] &&
        'c' == topic[1] &&
        'r' == topic[2] &&
        'e' == topic[3] &&
        'q' == topic[4] &&
        '/' == topic[5])
    {
        int i = 6;
        while (topic[i] != '/' && i < strlen(topic))
        {
            ++i;
        }
        if (i < strlen(topic))
            memcpy(g_cmdid, topic + i + 1, strlen(topic + i + 1));
    }
    return 0;
}

static int MqttSample_HandlePubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish ack, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubRec(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext *)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rec, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubRel(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext *)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rel, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubComp(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish comp, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandleSubAck(void *arg, uint16_t pkt_id, const char *codes, uint32_t count)
{
    uint32_t i;
    printf("Recv the subscribe ack, packet id is %d, return code count is %d:.\n", pkt_id, count);
    for (i = 0; i < count; ++i)
    {
        unsigned int code = ((unsigned char *)codes)[i];
        printf("   code%d=%02x\n", i, code);
    }

    return 0;
}

static int MqttSample_HandleUnsubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the unsubscribe ack, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandleCmd(void *arg, uint16_t pkt_id, const char *cmdid,
                                int64_t timestamp, const char *desc, const char *cmdarg,
                                uint32_t cmdarg_len, int dup, enum MqttQosLevel qos)
{
    uint32_t i;
    char cmd_str[100] = {0};
    struct MqttSampleContext *ctx = (struct MqttSampleContext *)arg;
    ctx->pkt_to_ack = pkt_id;
    strcpy(ctx->cmdid, cmdid);
    printf("Recv the command, packet id is %d, cmduuid is %s, qos=%d, dup=%d.\n",
           pkt_id, cmdid, qos, dup);

    if (0 != timestamp)
    {
        time_t seconds = timestamp / 1000;
        struct tm *st = localtime(&seconds);

        printf("    The timestampe is %04d-%02d-%02dT%02d:%02d:%02d.%03d.\n",
               st->tm_year + 1900, st->tm_mon + 1, st->tm_mday,
               st->tm_hour, st->tm_min, st->tm_sec, (int)(timestamp % 1000));
    }
    else
    {
        printf("    There is no timestamp.\n");
    }

    if (NULL != desc)
    {
        printf("    The description is: %s.\n", desc);
    }
    else
    {
        printf("    There is no description.\n");
    }

    printf("    The length of the command argument is %d, the argument is:", cmdarg_len);

    for (i = 0; i < cmdarg_len; ++i)
    {
        const char c = cmdarg[i];
        if (0 == i % 16)
        {
            printf("\n        ");
        }
        printf("%02X'%c' ", c, c);
    }
    printf("\n");
    memcpy(cmd_str, cmdarg, cmdarg_len);
    printf("cmd: %s\r\n", cmd_str);

    /* add your execution code here */
    /* 执行示例 -- 点亮led灯 */
    //LED_7_ON;

    return 0;
}

static int MqttSample_Subscribe(struct MqttSampleContext *ctx, char **topic, int num)
{
    int err;
    err = Mqtt_PackSubscribePkt(ctx->mqttbuf, 2, MQTT_QOS_LEVEL0, topic, num);
    if (err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the subscribe packet.\n");
        return -1;
    }
    return 0;
}

static int MqttSample_Unsubscribe(struct MqttSampleContext *ctx, char **topics, int num)
{
    int err;
    err = Mqtt_PackUnsubscribePkt(ctx->mqttbuf, 3, topics, num);
    if (err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the unsubscribe packet.\n");
        return -1;
    }

    return 0;
}

int MqttSample_Savedata11(struct MqttSampleContext *ctx, int temp, int humi)
{
    int err;
    struct MqttExtent *ext;
    uint16_t pkt_id = 1;

    char json[] = "{\"datastreams\":[{\"id\":\"temp\",\"datapoints\":[{\"value\":%d}]},{\"id\":\"humi\",\"datapoints\":[{\"value\":%d}]}]}";
    char t_json[200];
    int payload_len;
    char *t_payload;
    unsigned short json_len;

    sprintf(t_json, json, temp, humi);
    payload_len = 1 + 2 + strlen(t_json) / sizeof(char);
    json_len = strlen(t_json) / sizeof(char);

    t_payload = (char *)malloc(payload_len);
    if (t_payload == NULL)
    {
        printf("<%s>: t_payload malloc error\r\n", __FUNCTION__);
        return 0;
    }

    //type
    t_payload[0] = '\x01';

    //length
    t_payload[1] = (json_len & 0xFF00) >> 8;
    t_payload[2] = json_len & 0xFF;

    //json
    memcpy(t_payload + 3, t_json, json_len);

    if (ctx->mqttbuf->first_ext)
    {
        return MQTTERR_INVALID_PARAMETER;
    }

    printf("Topic: %s\r\nPakect ID: %d\r\nQoS: %d\r\nPayload: %s\r\n",
           "$dp", pkt_id, MQTT_QOS_LEVEL1, t_json);
    err = Mqtt_PackPublishPkt(ctx->mqttbuf, pkt_id, "$dp", t_payload, payload_len,
                              MQTT_QOS_LEVEL1, 0, 1);

    free(t_payload);

    if (err != MQTTERR_NOERROR)
    {
        return err;
    }

    return 0;
}

static int MqttSample_Savedata(struct MqttSampleContext *ctx, int temp, int humi)
{
    char opt;
    int Qos = 1;
    int type = 1;
    int i = 0;
    /*-q 0/1   ----> Qos0/Qos1
      -t 1/7   ----> json/float datapoint
    */

    printf("Qos: %d    Type: %d\r\n", Qos, type);
    MqttSample_Savedata11(ctx, temp, humi); // qos=1 type=1
}

static int MqttSample_Publish(struct MqttSampleContext *ctx, int latitude, int longitude)
{
    int err;
    int topics_len = 0;
    struct MqttExtent *ext;
    int i = 0;

    char *topic = "gps";
    char *payload1 = "%d, %d";
    char payload[100] = {0};
    int pkg_id = 1;

    sprintf(payload, payload1, latitude, longitude);
    printf("<%s>: public %s : %s\r\n", __FUNCTION__, topic, payload);
    if (ctx->mqttbuf->first_ext)
    {
        return MQTTERR_INVALID_PARAMETER;
    }
    err = Mqtt_PackPublishPkt(ctx->mqttbuf, pkg_id, topic, payload, strlen(payload), MQTT_QOS_LEVEL1, 0, 1);

    if (err != MQTTERR_NOERROR)
    {
        return err;
    }

    return 0;
}

/* 回复命令 */
int MqttSample_RespCmd(struct MqttSampleContext *ctx, char *resp)
{
    int err;
    int Qos = 0;
    int i = 0;

    printf("QoS: %d\r\nCmdId: %s\r\n", Qos, ctx->cmdid);

    if (0 == Qos)
    {
        err = Mqtt_PackCmdRetPkt(ctx->mqttbuf, 1, ctx->cmdid,
                                 resp, 11, MQTT_QOS_LEVEL0, 1);
    }
    else if (1 == Qos)
    {
        err = Mqtt_PackCmdRetPkt(ctx->mqttbuf, 1, ctx->cmdid,
                                 resp, 11, MQTT_QOS_LEVEL1, 1);
    }

    if (MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the cmd ret packet.\n");
        return -1;
    }

    return 0;
}

static int MqttSample_Init(struct MqttSampleContext *ctx)
{
    //struct epoll_event event;
    int err;

    ctx->host = MQTT_HOST;
    ctx->port = MQTT_PORT;
    ctx->sendedbytes = -1;

    ctx->devid = NULL;
    ctx->cmdid[0] = '\0';
    err = Mqtt_InitContext(ctx->mqttctx, 1000);
    if (MQTTERR_NOERROR != err)
    {
        printf("Failed to init MQTT context errcode is %d", err);
        return -1;
    }

    ctx->mqttctx->read_func = MqttSample_RecvPkt;
    ctx->mqttctx->writev_func = MqttSample_SendPkt;

    ctx->mqttctx->handle_conn_ack = MqttSample_HandleConnAck;
    ctx->mqttctx->handle_conn_ack_arg = ctx;
    ctx->mqttctx->handle_ping_resp = MqttSample_HandlePingResp;
    ctx->mqttctx->handle_ping_resp_arg = ctx;
    ctx->mqttctx->handle_publish = MqttSample_HandlePublish;
    ctx->mqttctx->handle_publish_arg = ctx;
    ctx->mqttctx->handle_pub_ack = MqttSample_HandlePubAck;
    ctx->mqttctx->handle_pub_ack_arg = ctx;
    ctx->mqttctx->handle_pub_rec = MqttSample_HandlePubRec;
    ctx->mqttctx->handle_pub_rec_arg = ctx;
    ctx->mqttctx->handle_pub_rel = MqttSample_HandlePubRel;
    ctx->mqttctx->handle_pub_rel_arg = ctx;
    ctx->mqttctx->handle_pub_comp = MqttSample_HandlePubComp;
    ctx->mqttctx->handle_pub_comp_arg = ctx;
    ctx->mqttctx->handle_sub_ack = MqttSample_HandleSubAck;
    ctx->mqttctx->handle_sub_ack_arg = ctx;
    ctx->mqttctx->handle_unsub_ack = MqttSample_HandleUnsubAck;
    ctx->mqttctx->handle_unsub_ack_arg = ctx;
    ctx->mqttctx->handle_cmd = MqttSample_HandleCmd;
    ctx->mqttctx->handle_cmd_arg = ctx;

    MqttBuffer_Init(ctx->mqttbuf);
    return 0;
}
int onenetGetDevID(struct MqttSampleContext *ctx)
{
    char device_id[20] = {0};
    char *p = NULL, *pend = NULL;
    sendHttpPkt(REG_PKT_HEAD, REG_PKT_BODY);
    mDelay(3000);
    /* 分析设备注册返回信息，获取设备id */
    if (0 != uwBuf_UnReadLen(pUartGPRS->pRsvbuf))
    {
        pSim800GPRS->Read((uint8_t *)device_id, 20);
        /* 获取 设备ID */
        if ((p = strstr(device_id, "device_id")) != NULL)
        {
            p += strlen("device_id\":\"");
            if ((pend = strstr(p, "\",")) != NULL)
            {
                memcpy(device_id, p, pend - p);
                printf("get device id: %s\r\n", device_id);
                *ctx->devid = device_id;
            }
        }
    }
    else if (ctx->devid == NULL)
    {
        printf("device regist fail!\r\n");
        while (1)
            ;
    }
}
/*连接mqtt主站*/
int onenetConnect()
{
    int err, bytes;
    /* MQTTcontext 初始化 */
    if (MqttSample_Init(onenetCtx) < 0)
    {
        return -1;
    }
    onenetGetDevID(onenetCtx);
    /****************初始化完成******************/
    /* mqtt连接 */
    MqttSample_Connect(onenetCtx, PROD_ID, API_KEY, onenetCtx->devid, KEEP_ALIVE, CLEAN_SESSION);
    bytes = Mqtt_SendPkt(onenetCtx->mqttctx, onenetCtx->mqttbuf, 0);
    MqttBuffer_Reset(onenetCtx->mqttbuf);
    mDelay(1000);
    err = Mqtt_RecvPkt(onenetCtx->mqttctx);
    return err;
}

int onenetSubscribe(char **topic, int num)
{
    int bytes;
    /* mqtt订阅 */
    MqttSample_Subscribe(onenetCtx, topics, 1); //可一次订阅多个，本例只用只订阅一个topic
    bytes = Mqtt_SendPkt(onenetCtx->mqttctx, onenetCtx->mqttbuf, 0);
    MqttBuffer_Reset(onenetCtx->mqttbuf);
    return bytes;
}
int onenetPublish(int latitude, int longtitude)
{
    int bytes;
    /*mqtt发布*/
    MqttSample_Publish(onenetCtx, latitude, longtitude);
    bytes = Mqtt_SendPkt(onenetCtx->mqttctx, onenetCtx->mqttbuf, 0);
    MqttBuffer_Reset(onenetCtx->mqttbuf);
    return bytes;
}
int onenetResCmd(char *resp)
{
    int bytes;
    /* mqtt回复命令 */
    MqttSample_RespCmd(onenetCtx, resp);
    bytes = Mqtt_SendPkt(onenetCtx->mqttctx, onenetCtx->mqttbuf, 0);
    MqttBuffer_Reset(onenetCtx->mqttbuf);
}
int onenetSendData(int temp, int humi)
{
    int bytes;
    /*mqtt上传数据 */
    MqttSample_Savedata(onenetCtx, temp, humi);
    bytes = Mqtt_SendPkt(onenetCtx->mqttctx, onenetCtx->mqttbuf, 0);
    MqttBuffer_Reset(onenetCtx->mqttbuf);
    return bytes;
}
/*mqtt接收数据*/
//err = Mqtt_RecvPkt(ctx->mqttctx);

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
