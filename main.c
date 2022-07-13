/*
file:
author:
date:
 */

#include <time.h>

#include "rtthread.h"
#include "RTT4/port/include/ls1b_gpio.h"
#include "RTT4/port/include/ls1b_irq.h"

#include "bsp.h"

#include "ls1x_rtc.h"

#include "ls1x_fb.h"
#include "ls1x-drv/include/spi/xpt2046.h"

#include "i2c/ads1015.h"
#include "i2c/drv_ads1015.h"

#include "gui/new_gui.h"
#include "src/datastructure_def.h"
#include "src/self_func.h"

char LCD_display_mode[] = LCD_800x480;

//-------------------------------------------------------------------------------------------------
#define PULSE_OUT 36
#define INPUT_SIGNAL 37 //单次抓取就位后，启动称量脉冲信号输入管脚
#define INPUT_SIGNAL2 34 //全部抓取就位后，启动称量脉冲信号输入管脚

#define VIL 0

rt_mq_t main_mq = RT_NULL;
rt_mq_t output_datas_mq = RT_NULL;
rt_mq_t file_write_mq = RT_NULL;
rt_mq_t nbiot_mq = RT_NULL;

rt_err_t uart3_input(rt_device_t dev, rt_size_t size);

int main(int argc, char **argv)
{
    ls1x_drv_init();         /* Initialize device drivers */
    rt_ls1x_drv_init();      /* Initialize device drivers for RTT */
    install_3th_libraries(); /* Install 3th libraies */

    do_touchscreen_calibrate(); /* 触摸屏校正 */

    rt_err_t ret;
    rt_mq_buf r_buf;

    res_paras res_t;
    product_info info_t;
    tr_stat stat_t;
    struct tm rtc_time;

    init_datastructure(&res_t, &info_t, &stat_t);
    init_rtc_time(&rtc_time, &info_t);

    unsigned char ctrl_flow = 0;
    uint16_t adc_sum = 0;
    unsigned char adc_measure_count = 0;
    // ui线程最先启动，准备好接收数据刷新到LCD
    output_datas_mq = rt_mq_create("output_datas_mq", 20, 4, RT_IPC_FLAG_FIFO);
    start_gui_task();
    send_v_threshold_msg(&res_t);
    send_rtc_msg(&info_t);

    uint16_t temp_adc;
    rt_device_t devADC;
    devADC = rt_device_find(ADS1015_DEVICE_NAME);
    if (devADC == NULL)
    {
        rt_kprintf("Open ADC device failed.\n");
        return -2;
    }
    rt_device_open(devADC, RT_DEVICE_FLAG_RDONLY);

    gpio_enable(PULSE_OUT, DIR_OUT);
    gpio_write(PULSE_OUT, VIL);

    ls1x_disable_gpio_interrupt(INPUT_SIGNAL);
    // EDGE_DOWN:下降沿触发
    // LEVLE_LOW:低电平触发
    ls1x_install_gpio_isr(INPUT_SIGNAL, INT_TRIG_EDGE_DOWN, input_interrupt_isr, 0);
    ls1x_enable_gpio_interrupt(INPUT_SIGNAL);

    ls1x_disable_gpio_interrupt(INPUT_SIGNAL2);
    ls1x_install_gpio_isr(INPUT_SIGNAL2, INT_TRIG_EDGE_DOWN, input_interrupt_isr2, 0);
    ls1x_enable_gpio_interrupt(INPUT_SIGNAL2);

    main_mq = rt_mq_create("main_mq", 20, 6, RT_IPC_FLAG_FIFO);
    if (main_mq != RT_NULL)
        rt_kprintf("create main_mq successfully.\n");
    file_write_mq = rt_mq_create("file_write_mq", 24, 2, RT_IPC_FLAG_FIFO);
    if (file_write_mq != RT_NULL)
        rt_kprintf("create file wr mq successfully.\n");
    nbiot_mq = rt_mq_create("nbiot_mq", 20, 2, RT_IPC_FLAG_FIFO);
    if (nbiot_mq != RT_NULL)
        rt_kprintf("create nb mq successfully.\n");
    start_file_wr_task();
    start_nb_task();
    while (1)
    {
        //所有动作均为消息或中断信号驱动，无须轮询
        ret = rt_mq_recv(main_mq, &r_buf.val_arr, 20, RT_WAITING_FOREVER);
        if (ret == RT_EOK)
        {
            switch (r_buf.val_arr[0])
            {
            case 1:
                //ctrl_flow为0表示未抓取6个电阻，此时不应该收到单次抓取就位信号，所以不需操作
                if (ctrl_flow)
                {
                    rt_device_read(devADC, ADS1015_CHANNEL_S3, (void *)&temp_adc, 2);
                    if (adc_sum >= temp_adc)
                        set_v_measure(&res_t, adc_sum - temp_adc);
                    else
                        set_v_measure(&res_t, 0);
                    adc_sum = temp_adc;
                    set_qualified(&res_t);
                    add_porduct(&info_t, &res_t);
                    set_output(&res_t);
                    send_data_msg(&res_t, &info_t);
                    set_msg_id(&stat_t);
                    send_nb_msg(&res_t, &stat_t);
                    rt_thread_delay(800);
                    gpio_write(PULSE_OUT, VIL);

                    adc_measure_count -= 1;
                    if (adc_measure_count == 0)
                    {
                        ctrl_flow = 0;
                        ls1x_enable_gpio_interrupt(INPUT_SIGNAL2);    //解除全部抓取就位脉冲信号屏蔽，防止误触发
                    }
                    ls1x_enable_gpio_interrupt(INPUT_SIGNAL);//置于此处消除信号毛刺
                }
                break;
            case 2: //写入空白日志文件
                send_info_to_file_msg(&info_t);
                send_date_to_file_msg(&info_t);
                rt_kprintf("got msg 2 from file wr thread.\n");
                break;
            case 3: //重启后恢复系统时间与统计数据
                set_product_info(&r_buf, &info_t);
                send_rtc_msg(&info_t);
                send_product_info_msg(&info_t);
                rt_kprintf("got msg 3 from file wr thread.\n");
                break;
            case 4: //
                set_csq(&r_buf, &stat_t);
                send_csq(&stat_t);
                break;
            case 5: //ͬ
                set_date_time(&r_buf, &info_t);
                send_rtc_msg(&info_t);
                break;
            case 6:
                res_t.v_threshold += 10;
                send_v_threshold_msg(&res_t);
                break;
            case 7:
                res_t.v_threshold -= 10;
                send_v_threshold_msg(&res_t);
                break;
            case 8:
                send_msg_id_msg(&stat_t);
                break;
            case 9:
                //抓取6个电阻时，脉冲触发回调发送消息
                if (ctrl_flow == 0)//防信号毛刺
                {
                    rt_device_read(devADC, ADS1015_CHANNEL_S3, (void *)&adc_sum, 2);
                    adc_measure_count = 6;
                    ctrl_flow = 1;
                }
                break;
            default:
                rt_kprintf("WARNNING!!!\n");
            }
            rt_kprintf("adc value:%i\n", temp_adc);
        }
        else
            rt_kprintf("receive error.\n");
    }
    return 0;
}

void input_interrupt_isr(int vector, void *param)
{
    static int op_code = 1;
    ls1x_disable_gpio_interrupt(INPUT_SIGNAL);
    rt_mq_send(main_mq, &op_code, sizeof(op_code));
}
void input_interrupt_isr2(int vector, void *param)
{
    static int op_code = 9;
    ls1x_disable_gpio_interrupt(INPUT_SIGNAL2);
    rt_mq_send(main_mq, &op_code, sizeof(op_code));
}
void init_datastructure(res_paras *r, product_info *p, tr_stat *t)
{
    r->v_measure = 0;
    r->v_threshold = 350;
    r->qualified = 0;

    p->total = 0;
    p->qualified_amount = 0;
    p->year = 2022;
    p->mon = 7;
    p->day = 13;

    t->csq = 0;
    t->msg_id = 0;
    t->stat_code = 0;
}
void init_rtc_time(rtc_obj *obj, product_info *p)
{
    obj->tm_year = p->year;
    obj->tm_mon = p->mon;
    obj->tm_mday = p->day;
    obj->tm_hour = 8;
    obj->tm_min = 0;
    obj->tm_sec = 0;
    ls1x_rtc_set_datetime(obj);
}

inline void set_v_measure(res_paras *r, unsigned short int t)
{
    r->v_measure = t * 2; //实际电压 = 测量数值 * 2
}

void set_qualified(res_paras *r)
{
    r->qualified = r->v_measure > r->v_threshold ? 1 : 0;
}
void add_porduct(product_info *p, res_paras *r)
{
    p->total += 1;
    p->qualified_amount += r->qualified;
}
void set_output(res_paras *r)
{
    gpio_write(PULSE_OUT, r->qualified); //合格：r->qualified为1
}
void send_v_threshold_msg(res_paras *r)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 2;
    sbuf.val_arr[1] = r->v_threshold;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 8);
}
void send_rtc_msg(product_info *p)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 3;
    sbuf.val_arr[1] = p->year;
    sbuf.val_arr[2] = p->mon;
    sbuf.val_arr[3] = p->day;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 16);
}
void send_data_msg(res_paras *r, product_info *p)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 1;
    sbuf.val_arr[1] = r->v_measure;
    sbuf.val_arr[2] = r->qualified;
    sbuf.val_arr[3] = p->total;
    sbuf.val_arr[4] = p->qualified_amount;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 20);
}
void send_product_info_msg(product_info *p)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 4;
    sbuf.val_arr[1] = p->total;
    sbuf.val_arr[2] = p->qualified_amount;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 12);
}

void set_product_info(rt_mq_buf *b, product_info *p)
{
    p->total = b->val_arr[1];
    p->qualified_amount = b->val_arr[2];
    p->year = b->val_arr[3];
    p->mon = b->val_arr[4];
    p->day = b->val_arr[5];
}
void set_csq(rt_mq_buf *b, tr_stat *t)
{
    t->csq = (b->asc_str[4] - '0') * 10 + b->asc_str[5] - '0';
}
void set_date_time(rt_mq_buf *b, product_info *p)
{
    p->year = (b->asc_str[4] - '0') * 1000 + (b->asc_str[5] - '0') * 100 +
              (b->asc_str[6] - '0') * 10 + (b->asc_str[7] - '0');
    p->mon = (b->asc_str[8] - '0') * 10 + (b->asc_str[9] - '0');
    p->day = (b->asc_str[10] - '0') * 10 + (b->asc_str[11] - '0');
}
void set_msg_id(tr_stat *t)
{
    t->msg_id += 1;
    if (t->msg_id > 999)
        t->msg_id = 0;
}
void send_csq(tr_stat *t)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 5;
    sbuf.val_arr[1] = t->csq;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 8);
}

void send_msg_id_msg(tr_stat *t)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 6;
    sbuf.val_arr[1] = t->msg_id;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 8);
}
void send_stat_code_msg(tr_stat *t)
{
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 7;
    sbuf.val_arr[1] = 200;
    rt_mq_send(output_datas_mq, &sbuf.val_arr, 8);
}

void send_nb_msg(res_paras *r, tr_stat *t)
{
    //将数值转换为字符串，使用rt_sprintf也可
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 1;
    sbuf.asc_str[4] = (r->v_measure / 1000) + '0';
    sbuf.asc_str[5] = ((r->v_measure % 1000) / 100) + '0';
    sbuf.asc_str[6] = (r->v_measure % 100) / 10 + '0';
    sbuf.asc_str[7] = r->v_measure % 10 + '0';

    sbuf.asc_str[8] = r->qualified + '0';

    sbuf.asc_str[9] = t->msg_id / 100 + '0';
    sbuf.asc_str[10] = (t->msg_id % 100) / 10 + '0';
    sbuf.asc_str[11] = t->msg_id % 10 + '0';
    sbuf.asc_str[12] = r->v_threshold / 1000 + '0';
    sbuf.asc_str[13] = (r->v_threshold % 100) / 100 + '0';
    sbuf.asc_str[14] = (r->v_threshold % 100) / 10 + '0';
    sbuf.asc_str[15] = r->v_threshold % 10 + '0';
    rt_mq_send(nbiot_mq, &sbuf.asc_str, 16);
}

void send_info_to_file_msg(product_info *p){
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 1;
    subf.val_arr[1] = p->total;
    sbuf.val_arr[2] = p->qualified_amount;
    rt_mq_send(file_write_mq, &sbuf.asc_str, 12);
}
void send_date_to_file_msg(product_info *p){
    rt_mq_buf sbuf;
    sbuf.val_arr[0] = 2;
    subf.val_arr[1] = p->year;
    sbuf.val_arr[2] = p->mon;
    sbuf.val_arr[3] = p->day;
    rt_mq_send(file_write_mq, &sbuf.asc_str, 16);
}

/*
 * @@ End
 */
