
#include "uart.h"
#include "utils.h"
#include "sht20.h"
#include "hal_i2c.h"

#define PROD_ID "173278"            //修改为自己的产品ID
#define SN "201608160002"           //修改为自己的设备唯一序列号
#define REG_CODE "6TM7OkhNsTjATvFx" //修改为自己的产品注册码
#define API_ADDR "api.heclouds.com"

#define DEVICE_NAME "kylin_" SN

#define REG_PKT_HEAD "POST http://" API_ADDR "/register_de?register_code=" REG_CODE " HTTP/1.1\r\n" \
                     "Host: " API_ADDR "\r\n"                                                       \
                     "api-key:reC6LwIXkj6j9=AV5CCo8g6qijg="
//"Content-Length: "

#define REG_PKT_BODY "{\"title\":\"" DEVICE_NAME "\"}"

#include "mqtt.h"

#define STRLEN 64
char g_cmdid[STRLEN];

struct MqttSampleContext
{
    //    int epfd;
    //    int mqttfd;
    uint32_t sendedbytes;
    struct MqttContext mqttctx[1];
    struct MqttBuffer mqttbuf[1];

    const char *host;
    unsigned short port;

    const char *proid;
    const char *devid;
    const char *apikey;

    int dup;
    enum MqttQosLevel qos;
    int retain;

    uint16_t pkt_to_ack;
    char cmdid[70];
};

void sendHttpPkt(char *phead, char *pbody)
{
    char sendBuf0[20];
    char sendBuf1[500];

    sprintf(sendBuf1, "%s%d\r\n\r\n%s", phead, strlen(pbody), pbody);
    printf("send HTTP pkt:\r\n%s\r\n", sendBuf1);

    sprintf(sendBuf0, "AT+CIPSEND=%d\r\n", strlen(sendBuf1));
    SendCmd(sendBuf0, ">", 500);
    USART2_Clear();

    /* EDP设备连接包，发送 */
    USART2_Write(USART2, (uint8_t *)sendBuf1, strlen(sendBuf1)); //串口发送
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
  * @brief  非透传模式发送EDP报文
  */
void sendPkt(char *p, int len)
{
    char sendBuf[30] = {0};

    /* 非透传模式先发送AT+CIPSEND=X */
    sprintf(sendBuf, "AT+CIPSEND=%d\r\n", len);
    SendCmd(sendBuf, ">", 500);
    USART2_Clear();

    /* EDP设备连接包，发送 */
    USART2_Write(USART2, p, len); //串口发送
    //hexdump(p, len);     //打印发送内容
}

//-------------------------------- Commands ------------------------------------------------------
static int MqttSample_Connect(struct MqttSampleContext *ctx, char *proid, char *auth_info, const char *devid, int keep_alive, int clean_session)
{
    int err, flags;
    //    struct epoll_event event;

    //     if(ctx->mqttfd >= 0) {
    //         close(ctx->mqttfd);
    //         epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, ctx->mqttfd, NULL);
    //     }

    //     ctx->mqttfd = MqttSample_CreateTcpConnect(ctx->host, ctx->port);
    //     if(ctx->mqttfd < 0) {
    //         return -1;
    //     }
    //     ctx->mqttctx->read_func_arg = (void*)(size_t)ctx->mqttfd;
    //     ctx->mqttctx->writev_func_arg = (void*)(size_t)ctx->mqttfd;

    //     flags = fcntl(ctx->mqttfd, F_GETFL, 0);
    // 	if(-1 == flags) {
    // 	    printf("Failed to get the socket file flags, errcode is %d.\n", errno);
    // 	}
    //
    //     if(fcntl(ctx->mqttfd, F_SETFL, flags | O_NONBLOCK) < 0) {
    //         printf("Failed to set the socket to nonblock mode, errcode is %d.\n", errno);
    //         return -1;
    //     }

    //     event.data.fd = ctx->mqttfd;
    //     event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
    //     if(epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->mqttfd, &event) < 0) {
    //         printf("Failed to add the socket to the epoll, errcode is %d.\n", errno);
    //         return -1;
    //     }

    //ctx->apikey = "F8E2ABB4278D47188CF6C1B3741D0DA1"; //discard
    //ctx->proid = "1234";  //discard

    printf("product id: %s\r\nsn: %s\r\ndeviceid: %s\r\nkeepalive: %d\r\ncleansession: %d\r\nQoS: %d\r\n",
           proid, auth_info, devid, keep_alive, clean_session, MQTT_QOS_LEVEL0);
    err = Mqtt_PackConnectPkt(ctx->mqttbuf, keep_alive, devid,
                              clean_session, NULL,
                              NULL, 0,
                              MQTT_QOS_LEVEL0, 0, proid,
                              auth_info, strlen(auth_info));
    /*
    err = Mqtt_PackConnectPkt(ctx->mqttbuf, keep_alive, ctx->devid, 1, user_name,
                              password, sizeof(password), MQTT_QOS_LEVEL0, 0, NULL,
                              NULL, 0);
    */
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

    /* 完成串口数据包的接收 */
    while ((rcv_len = USART2_GetRcvNum()) > 0)
    {
        printf("rcv_len: %d\r\n", rcv_len);
        //        hexdump(usart2_rcv_buf, usart2_rcv_len);   //打印接收数据
        mDelay(500);
    }

    if (usart2_rcv_len != 0) //连同wifi的AT指令返回信息
    {
        USART2_GetRcvData(buffer, usart2_rcv_len);
        /* 串口数据分析，区分WIFI模块的推送信息与服务器的下发信息 */
        if ((p = uartDataParse(buffer, &onenetdata_len)) == NULL)
        {
            //printf("No server Data\r\n");
            /* 无有效数据，返回0 */
            return 0;
        }
        /* 成功接收了rcv_len个字节的数据，将其写入recv_buf，用于解析 */
        //WriteBytes(recv_buf, p, rcv_len);
        memcpy(buf, p, onenetdata_len);

        if (onenetdata_len > 0)
        {
            printf("Rcv: \r\n");
            for (i = 0; i < onenetdata_len; i++)
            {
                printf("%02X ", ((unsigned char *)buf)[i]);
            }
            printf("\r\n");
        }
    }
    return onenetdata_len;
}

static int MqttSample_SendPkt(void *arg, const struct iovec *iov, int iovcnt)
{
    char sendbuf[1024];
    int len = 0;
    int bytes;
    //struct msghdr msg;
    //memset(&msg, 0, sizeof(msg));
    //msg.msg_iov = (struct iovec*)iov;
    //msg.msg_iovlen = (size_t)iovcnt;

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

    //bytes = sendmsg((int)(size_t)arg, &msg, 0);
    return bytes;
}

//------------------------------- packet handlers -------------------------------------------
static int MqttSample_HandleConnAck(void *arg, char flags, char ret_code)
{
    LED_4_ON;
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
    LED_7_ON;

    return 0;
}

static int MqttSample_Subscribe(struct MqttSampleContext *ctx, char **topic, int num)
{
    int err;
    //    char **topics;
    //    int topics_len = 0;

    //    topics = str_split(buf, ' ', &topics_len);

    //    if (topics){
    //        int i;
    //        for (i = 0; *(topics + i); i++){
    //                    printf("%s\n", *(topics + i));
    //        }
    //        printf("\n");
    //     }

    //sprintf(topic, "%s/%s/45523/test-1", ctx->proid, ctx->apikey);
    err = Mqtt_PackSubscribePkt(ctx->mqttbuf, 2, MQTT_QOS_LEVEL0, topic, num);
    if (err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the subscribe packet.\n");
        return -1;
    }

    /*
    sprintf(topic, "%s/%s/45523/test-2", ctx->proid, ctx->apikey);
    err = Mqtt_AppendSubscribeTopic(ctx->mqttbuf, topic, MQTT_QOS_LEVEL1);
    if (err != MQTTERR_NOERROR) {
        printf("Critical bug: failed to pack the subscribe packet.\n");
        return -1;
    }
    */

    return 0;
}

static int MqttSample_Unsubscribe(struct MqttSampleContext *ctx, char **topics, int num)
{
    int err;
    //     char **topics;
    //     int topics_len = 0;
    //     topics = str_split(buf, ' ', &topics_len);

    //     printf("topic len %d\n", topics_len);
    //     if (topics){
    //         int i;
    //         for (i = 0; *(topics + i); i++){
    //                     printf("%s\n", *(topics + i));
    //         }
    //         printf("\n");
    //     }

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

static int MqttSample_Publish(struct MqttSampleContext *ctx, int temp, int humi)
{
    int err;
    int topics_len = 0;
    struct MqttExtent *ext;
    int i = 0;

    char *topic = "key_press";
    char *payload1 = "key pressed, temp: %d, humi: %d";
    char payload[100] = {0};
    int pkg_id = 1;

    sprintf(payload, payload1, temp, humi);
    printf("<%s>: public %s : %s\r\n", __FUNCTION__, topic, payload);

    /*??????????

    topics = str_split(buf, ' ', &topics_len);

	printf("topics_len: %d\r\n", topics_len);

    if (topics){
        int i;
        for (i = 0; *(topics + i); i++){
                    printf("%s\n", *(topics + i));
        }
        printf("\n");
    }
    if(4 != topics_len){
        printf("usage:push_dp topicname payload pkg_id");
        return err;
    }

 */
    if (ctx->mqttbuf->first_ext)
    {
        return MQTTERR_INVALID_PARAMETER;
    }

    /*
    std::string pkg_id_s(topics+3);
    int pkg_id = std::stoi(pkg_id_s);
    */

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

    //    ctx->mqttfd = -1;

    /*
    ctx->host = "192.168.200.218";
    ctx->port = 6002;
    ctx->proid = "433223";
    ctx->devid = "45523";
    ctx->apikey = "Bs04OCJioNgpmvjRphRak15j7Z8=";
    */

    err = Mqtt_InitContext(ctx->mqttctx, 1000);
    if (MQTTERR_NOERROR != err)
    {
        printf("Failed to init MQTT context errcode is %d", err);
        return -1;
    }

    ctx->mqttctx->read_func = MqttSample_RecvPkt;
    //    ctx->mqttctx->read_func_arg =  (void*)(size_t)ctx->mqttfd;
    //    ctx->mqttctx->writev_func_arg =  (void*)(size_t)ctx->mqttfd;
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

    //     ctx->epfd = epoll_create(10);
    //     if(ctx->epfd < 0) {
    //         printf("Failed to create the epoll instance.\n");
    //         return -1;
    //     }

    //     if(fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) < 0) {
    //         printf("Failed to set the stdin to nonblock mode, errcode is %d.\n", errno);
    //         return -1;
    //     }

    //     event.data.fd = STDIN_FILENO;
    //     event.events = EPOLLIN;
    //     if(epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0) {
    //         printf("Failed to add the stdin to epoll, errcode is %d.\n", errno);
    //         return -1;
    //     }

    return 0;
}

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //使能GPIOC时钟

    /* 功能键 SW2--PC11 SW3--PC12 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50M
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //初始化PC11 PC12

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); //使能GPIOC时钟
    /* 功能键 SW4--PD2  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50M
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //初始化PD2
}

int KEY2_Scan(void)
{
    if (KEY2_UP == 0)
    {
        mDelay(10); //去抖动
        if (KEY2_UP == 0)
            return 0;
    }

    return 1;
}

int KEY3_Scan(void)
{
    if (KEY3_UP == 0)
    {
        mDelay(10); //去抖动
        if (KEY3_UP == 0)
            return 0;
    }
    return 1;
}

/**
  * @brief     实现EDP连接，心跳包保持在线，接收透传消息并返回
  * @attention 使用UART2连接ESP8266模块，使用非透传模式发送和接收数据
  *			   使用UART1作为调试打印串口，使用printf将从该接口打印消息
  *			   Recv_Thread_Func函数是调用的官方edp_sdk中提供的函数，在其上加以修改
  *			   其中提供了所有的EDP包的解析函数
  */
int main(void)
{
    uint16_t temp, humi; //温湿度
    uint32_t timeCount = 0;
    uint32_t saveDataCount = 0, publishCount = 0;
    int num = 0, err;
    char buf[100];
    char device_id[20] = {0};
    char *topics[] = {"test_topic", "test_topic2"};

    /* MQTT 相关 */
    struct MqttSampleContext ctx[1];
    int bytes;

    /* mqtt连接参数 */
    int keep_alive = 120;
    int clean_session = 0;
    //   ctx->devid = "3264518";  //device_id

    USART1_Config(); //USART1作为打印串口
    USART2_Config(); //USART2连接ESP8266模块
    KEY_Init();
    LED_Init();
    Hal_I2C_Init(); //I2C初始化，用于连接温湿度传感器

    LED_4_ON;
    LED_5_ON;
    LED_6_ON;
    LED_7_ON;
    mDelay(2000);
    LED_4_OFF;
    LED_5_OFF;
    LED_6_OFF;
    LED_7_OFF;

#if 1
    /* MQTTcontext 初始化 */
    if (MqttSample_Init(ctx) < 0)
    {
        return -1;
    }

init_8266:
    /* 8266初始化 */
    SendCmd(AT, "OK", 1000);     //模块有效性检查
    SendCmd(CWMODE, "OK", 1000); //模块工作模式
    SendCmd(CIFSR, "OK", 1000);  //查询网络信息
    SendCmd(CWJAP, "OK", 2000);  //配置需要连接的WIFI热点SSID和密码
    //LED_4_ON;

    SendCmd(CIPMODE0, "OK", 1000);      //配置非透传模式
    SendCmd(HTTP_CIPSTART, "OK", 2000); //HTTP_TCP连接

    USART2_Clear();

    sendHttpPkt(REG_PKT_HEAD, REG_PKT_BODY);
    mDelay(3000);
    /* 分析设备注册返回信息，获取设备id */
    if (usart2_rcv_len > 0)
    {
        char *p = NULL, *pend = NULL;
        printf("rcv response:\r\n%s\r\n", usart2_rcv_buf);

        /* 获取 设备ID */
        if ((p = strstr(usart2_rcv_buf, "device_id")) != NULL)
        {
            p += strlen("device_id\":\"");
            if ((pend = strstr(p, "\",")) != NULL)
            {
                memcpy(device_id, p, pend - p);
                printf("get device id: %s\r\n", device_id);
                ctx->devid = device_id;
            }
        }
    }

    else if (usart2_rcv_len == 0)
    {
        printf("device regist over time!\r\n");
        while (1)
            ;
    }
    else if (ctx->devid == NULL)
    {
        printf("device regist fail!\r\n");
        while (1)
            ;
    }

    SendCmd(CIPCLOSE, "OK", 2000);      //断开TCP连接
    SendCmd(MQTT_CIPSTART, "OK", 2000); //MQTT_TCP连接
    //while(1);
#endif
    /****************初始化完成******************/

    USART2_Clear();

#if 1
    /* mqtt连接 */
    MqttSample_Connect(ctx, PROD_ID, SN, ctx->devid, keep_alive, clean_session);
    bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    mDelay(1000);
    err = Mqtt_RecvPkt(ctx->mqttctx);
    USART2_Clear();

    /* mqtt订阅 */
    MqttSample_Subscribe(ctx, topics, 1); //可一次订阅多个，本例只用只订阅一个topic
    bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    mDelay(1000);
#endif
    while (1)
    {
        if (KEY2_Scan() == 0)
        {
            printf("key2 pressed\r\n");
            LED_5_ON;
            mDelay(1000);

            /* 发布消息 */
            MqttSample_Publish(ctx, temp, humi);
            //           bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
            //           MqttBuffer_Reset(ctx->mqttbuf);
            bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
            MqttBuffer_Reset(ctx->mqttbuf);
            LED_5_OFF;
        }
        if (KEY3_Scan() == 0)
        {
            printf("key3 pressed\r\n");
            if (ctx->cmdid[0] != '\0') //若已经接收到命令
            {
                /* 回复命令 */
                MqttSample_RespCmd(ctx, "cmd received");
                bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
                MqttBuffer_Reset(ctx->mqttbuf);

                ctx->cmdid[0] = '\0';
            }
        }

        if (saveDataCount >= 300)
        {
            LED_6_ON;
            saveDataCount = 0;

            /*  */
            SHT2x_MeasureHM(SHT20_Measurement_T_HM, &temp);
            SHT2x_MeasureHM(SHT20_Measurement_RH_HM, &humi);
            printf("temp: %d, humi: %d\r\n", temp, humi);

            /* 上传数据 */
            MqttSample_Savedata(ctx, temp, humi);
            //            bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
            //            MqttBuffer_Reset(ctx->mqttbuf);
            bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
            MqttBuffer_Reset(ctx->mqttbuf);

            LED_6_OFF;
        }

        err = Mqtt_RecvPkt(ctx->mqttctx);
        USART2_Clear();
        timeCount++;
        saveDataCount++;
        publishCount++;
        mDelay(100);
        //printf(".");
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
