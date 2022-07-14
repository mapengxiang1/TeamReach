/*
file:nb_conn.c
author:teamreach
date:2022.7.6
Functions to send AT commands to NB-IoT module for getting connection with TCP server and  publishing thing module topics via MQTT protocol.
 */
#include "bsp.h"
#include "rtthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "app_os_priority.h"
#include "../src/datastructure_def.h"
//#define NB_IOT_MODULE
#ifdef NB_IOT_MODULE
#include "../src/at_command.h"
#else
const char* at_bare="AT\r\n";
const char* ate0="ATE0\r\n";//取消回显
//TCP连接指令
const char* at_csoc="AT+CSOC=1,1,1\r\n";
const char* at_csocon="AT+CSOCON=0,{PORT},\"{IP ADDRESS}\"\r\n";
const char* at_csocl="AT+CSOCL=0\r\n";
//MQTT连接指令
const char* at_cmqnew="AT+CMQNEW=\"{ProductKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com\",\"1883\",12000,1024\r\n";
const char* at_cmqalicfg="AT+CMQALICFG=0,\"{ProductKey}\",\"{DeviceName}\",\"{DeviceSecret}\"\r\n";
const char* at_cmqalicon="AT+CMQALICON=0,666,1\r\n";
char* at_cmqpub ="AT+CMQPUB=0,\"/sys/{ProductKey}/{DeviceName}/thing/event/property/post\",1,0,0,103,\"{JSON String}\"\r\n";
const char* at_cmqdiscon="AT+CMQDISCON=0\r\n";
#endif

extern rt_mq_t main_mq;
extern rt_mq_t nbiot_mq;

void nb_comm(void *args);
int nb_init(rt_device_t s);
int nb_get_csq(rt_device_t s, rt_mq_buf *mqbuf);
int nb_get_time(rt_device_t s, rt_mq_buf *mqbuf);
int nb_send_v_q(rt_device_t s, rt_mq_buf *mqbuf);
void set_at_cmd(char *, rt_mq_buf *);
void send_nb_conn_msg(rt_mq_buf *r);

int start_nb_task(void)
{
    rt_thread_t nb_thread = rt_thread_create("nb_thread", nb_comm, NULL, I2C0_STK_SIZE * 2, 17, 10);
    if (nb_thread == NULL)
    {
        rt_kprintf("create nbiot thread failed.\n");
        return -1;
    }
    rt_thread_startup(nb_thread);
    rt_kprintf("create nb thread succeed.\n");
    return 0;
}
void nb_comm(void *args)
{
    rt_err_t ret;
    rt_device_t serial;
    rt_mq_buf mqbuf;
    serial = rt_device_find("uart3");
    if (!serial)
        rt_kprintf("device not found.\n");
    else
    {
        ret = rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
        if (ret == RT_EOK)
        {
            ret = nb_init(serial);
            ret = nb_get_csq(serial, &mqbuf);
            ret = nb_get_time(serial, &mqbuf);
            while (1)
            {
                rt_kprintf("keep waiting for msg form main thread.\n");
                ret = rt_mq_recv(nbiot_mq, &mqbuf.val_arr, 24, RT_WAITING_FOREVER);
                if (ret == RT_EOK)
                {
                    switch (mqbuf.val_arr[0])
                    {
                    case 1: //发送单次测量电压、阈值电压和判定结果
                        set_at_cmd(at_cmqpub, &mqbuf);
                        if (!nb_send_v_q(serial, &mqbuf))
                            //通知main thread 发送成功
                            send_nb_conn_msg(&mqbuf);
                        break;
                    default:
                        break;
                    }
                }
            }
            rt_device_close(serial);
        }
        else
            rt_kprintf("open serial device failed.\n");
    }
}
inline int parse_ok(char *s, rt_size_t l)
{
    //判断回复长度是否大于4，内容以“OK”结束且以“\r\n”结尾
    return (l >= 4 && s[l - 1] == '\n' && s[l - 2] == '\r' && s[l - 3] == 'K' && s[l - 4] == 'O') ? 1 : 0;
}
int nb_init(rt_device_t s)
{
    rt_size_t length;
    char buf[20];
    rt_device_write(s, 0, "AT\r\n", 4);
    rt_thread_delay(100);
    length = rt_device_read(s, -1, buf, 16);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("test AT cmd failed.\n");
        return -1;
    }
    rt_device_write(s, 0, "ATE0\r\n", 6);
    rt_thread_delay(100);
    length = rt_device_read(s, -1, buf, 16);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("test ATE0 cmd failed.\n");
        return -1;
    }
    return 0;
}
int nb_get_csq(rt_device_t s, rt_mq_buf *mqbuf)
{
    rt_size_t length;
    char buf[40];
    mqbuf->val_arr[0] = 4;
    rt_device_write(s, 0, "AT+CSQ\r\n", 8);
    rt_thread_delay(100);
    length = rt_device_read(s, -1, buf, 20);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("get CSQ  failed.\n");
        return -1;
    }
    mqbuf->asc_str[4] = buf[8];
    mqbuf->asc_str[5] = buf[9];
    rt_mq_send(main_mq, mqbuf->asc_str, 6);
    return 0;
}

int nb_get_time(rt_device_t s, rt_mq_buf *mqbuf)
{
    rt_size_t length;
    char buf[60];
    mqbuf->val_arr[0] = 5;
    rt_device_write(s, 0, at_csoc, 15);
    rt_thread_delay(100);
    length = rt_device_read(s, -1, buf, 20);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("at csoc  failed.\n");
        return -1;
    }
    rt_device_write(s, 0, at_csocon, 33);
    rt_thread_delay(1800);
    length = rt_device_read(s, -1, buf, 50);
    if (length != 48)
    {
        rt_kprintf("csocon failed.\n");
        return -1;
    }
    //服务器回复时间格式为“年月日”，例如“20220713”，
    mqbuf->asc_str[4] = buf[23];
    mqbuf->asc_str[5] = buf[25];
    mqbuf->asc_str[6] = buf[27];
    mqbuf->asc_str[7] = buf[29];
    mqbuf->asc_str[8] = buf[31];
    mqbuf->asc_str[9] = buf[33];
    mqbuf->asc_str[10] = buf[35];
    mqbuf->asc_str[11] = buf[37];
    rt_device_write(s, 0, at_csocl, 12);
    rt_mq_send(main_mq, mqbuf->asc_str, 12);

    return 0;
}

int nb_send_v_q(rt_device_t s, rt_mq_buf *mqbuf)
{
    rt_size_t length;
    char buf[60];
    static unsigned char flag = 0;
    //连接状态标志位，成功建立连接后不需要再初始化、配置信息、建立连接。
    if (flag == 0)
    {

        rt_device_write(s, 0, at_cmqdiscon, 16);

        rt_device_write(s, 0, at_cmqnew, 80);
        rt_thread_delay(2000);
        length = rt_device_read(s, -1, buf, 40);
        if (!parse_ok(buf, length))
        {
            rt_kprintf("at cmqnew  failed.\n");
            return -1;
        }

        rt_device_write(s, 0, at_cmqalicfg, 73);
        rt_thread_delay(1000);
        length = rt_device_read(s, -1, buf, 40);
        if (!parse_ok(buf, length))
        {
            rt_kprintf("at cmqalicfg  failed.\n");
            return -1;
        }

        rt_device_write(s, 0, at_cmqalicon, 22);
        rt_thread_delay(1000);
        length = rt_device_read(s, -1, buf, 40);
        if (!parse_ok(buf, length))
        {
            rt_kprintf("at cmqalicon  failed.\n");
            return -1;
        }
        flag = 1;
    }
    //无实际功能，相当于刷新状态，防止后续指令出错
    rt_device_write(s, 0, "AT\r\n", 4);
    rt_thread_delay(100);
    length = rt_device_read(s, -1, buf, 16);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("test AT cmd failed.\n");
    }

    rt_device_write(s, 0, at_cmqpub, 199);
    rt_thread_delay(2000);
    length = rt_device_read(s, -1, buf, 60);
    if (!parse_ok(buf, length))
    {
        rt_kprintf("at cmqpub  failed.\n");
        flag = 0;
        return -1;
    }
    else
        flag = 1;
    return 0;
}

void set_at_cmd(char *a, rt_mq_buf *r)
{
    //替换JSON String中对应ID的值所转化成的字符串
    a[84] = r->asc_str[9];
    a[85] = r->asc_str[10];
    a[86] = r->asc_str[11];
    //替换JSON String中对应属性电压测量值的值所转化成的字符串
    a[128] = r->asc_str[4];
    a[130] = r->asc_str[5];
    a[131] = r->asc_str[6];
    a[132] = r->asc_str[7];
    //替换JSON String中对应属性判定结果的值所转化成的字符串
    a[140] = r->asc_str[8];
   //替换JSON String中对应属性电压阈值的值所转化成的字符串
    a[149] = r->asc_str[12];
    a[151] = r->asc_str[13];
    a[152] = r->asc_str[14];
    a[153] = r->asc_str[15];
}
inline void send_nb_conn_msg(rt_mq_buf *r)
{
    r->val_arr[0] = 8;
    rt_mq_send(main_mq, r->val_arr, 4);
}

/*
 * @END
 */
