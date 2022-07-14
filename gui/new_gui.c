/*
file:new_gui.c
author:teamreach
date:2022.7.6
部分函数从样例程序demo_gui.c文件中拷贝，原作者Bian
 */
#include "bsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls1x_fb.h"
#include "spi/xpt2046.h"
#include "simple-gui/simple_gui.h"

#include "app_os_priority.h"
#include "../src/datastructure_def.h"

#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 42

extern rt_mq_t output_datas_mq;
extern rt_mq_t main_mq;

int start_flush_task(void);
extern rt_err_t rt_thread_sleep(rt_tick_t);
void rt_flush_task(void *arg);

void draw_text_in_grid(int row, int col, unsigned int *val);
void draw_text_in_grid_nb(int row, int col, unsigned int *val, char *buf);
void draw_texts_in_grids(rt_mq_buf *);
// void draw_v_threshold_in_grid(rt_mq_buf*);
void draw_rtc_time_in_grid(int row, int col, rt_mq_buf *);
void draw_product_info(rt_mq_buf *buf);

void send_btn_msg(unsigned int);
/******************************************************************************
 * show message
 * 原作者Bian
 ******************************************************************************/

static volatile int last_x = -1;
static volatile int last_y = -1;

void gui_info(const char *str, int x, int y)
{
    if ((str == NULL) || (x < 0) || (y < 0))
        return;

    /* clear last */
    if ((last_x > 0) && (last_y > 0))
    {
        FillRect(last_x, last_y, fb_get_pixelsx() - 1, last_y + 16, cidxBLACK);
    }

    PutString(x, y, (char *)str, cidxBRTWHITE);

    last_x = x;
    last_y = y;
}

/******************************************************************************
 * defined functions
 * 原作者Bian
 ******************************************************************************/

int clear_screen(void)
{
    /* fill screen color silver
     */
    ls1x_dc_ioctl(devDC, IOCTRL_FB_CLEAR_BUFFER, (void *)GetColor(cidxSILVER));

    return 0;
}

static int create_main_objects(void);

static void set_objects_active_group(int group);
/******************************************************************************
 * main frame
 * 原作者Bian
 ******************************************************************************/

#define MAIN_GROUP 0x00010000

static TGrid *grid_main = NULL;

static int create_grid_main(void)
{
    TColumn *p_column;
    TRect rect;

    rect.left = 100;
    rect.top = 160;

    rect.right = 700;
    rect.bottom = 300;
    grid_main = create_grid(&rect, 3, 6, MAIN_GROUP | 0x0001, MAIN_GROUP);

    if (grid_main == NULL)
        return -1;

    /* column 0 */
    p_column = grid_get_column(grid_main, 0);
    p_column->align = align_center;
    grid_set_column_title(p_column, "产品参数");
    grid_set_column_width(grid_main, 0, 100);

    /* column 1 */
    p_column = grid_get_column(grid_main, 1);
    p_column->align = align_center;
    grid_set_column_title(p_column, "数值ֵ");
    grid_set_column_width(grid_main, 1, 100);
     /* column 2 */
    p_column = grid_get_column(grid_main, 2);
    p_column->align = align_center;
    grid_set_column_title(p_column, "生产指标");
    grid_set_column_width(grid_main, 2, 100);

    /* column 3 */
    p_column = grid_get_column(grid_main, 3);
    p_column->align = align_center;
    grid_set_column_title(p_column, "数值ֵ");
    grid_set_column_width(grid_main, 3, 100);
    /* column 4 */
    p_column = grid_get_column(grid_main, 4);
    p_column->align = align_center;
    grid_set_column_title(p_column, "ͨ通信参数");
    grid_set_column_width(grid_main, 4, 100);
    /* column 5 */
    p_column = grid_get_column(grid_main, 5);
    p_column->align = align_center;
    grid_set_column_title(p_column, "数值/码");
    grid_set_column_width(grid_main, 5, 100);

    return 0;
}
/******************************************************************************
 *callback functions
 * 已修改，原作者Bian
 ******************************************************************************/
static void main_onclick_0001(unsigned msg, void *param)
{
    send_btn_msg(6);
}

static void main_onclick_0002(unsigned msg, void *param)
{
    send_btn_msg(7);
}

static void main_onclick_0003(unsigned msg, void *param)
{
    // TODO
}

static void main_onclick_0004(unsigned msg, void *param)
{
}

static int create_buttons_main(void)
{
    TRect rect;

    rect.left = 100;
    rect.top = 360;
    rect.right = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, MAIN_GROUP | 0x0001, MAIN_GROUP, "增大阈值", main_onclick_0001);

    rect.left = 260;
    rect.top = 360;
    rect.right = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, MAIN_GROUP | 0x0002, MAIN_GROUP, "减小阈值ֵ", main_onclick_0002);

    rect.left = 420;
    rect.top = 360;
    rect.right = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, MAIN_GROUP | 0x0003, MAIN_GROUP, "恢复出厂", main_onclick_0003);

    rect.left = 580;
    rect.top = 360;
    rect.right = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, MAIN_GROUP | 0x0004, MAIN_GROUP, "功能预留", main_onclick_0004);

    return 0;
}

static int create_main_objects(void)
{
    if (clear_screen() != 0)
        return -1;

    if (get_buttons_count(MAIN_GROUP) == 0)
        create_buttons_main();

    if (grid_main == NULL)
        create_grid_main();

    set_objects_active_group(MAIN_GROUP);

    return 0;
}

/******************************************************************************
 * 表格绘制API
 * 已修改，原作者Bian
 ******************************************************************************/

static void set_objects_active_group(int group)
{
    set_gui_active_group(group);
}

void gui_drawtext_in_grid(int row, int col, const char *str)
{
    if (!ls1x_dc_started())
        return;

    if (grid_main != NULL)
        grid_set_cell_text(grid_main, row, col, str);
}

void grid_content_init()
{
    gui_drawtext_in_grid(0, 0, "电压");
    gui_drawtext_in_grid(1, 0, "阈值");
    gui_drawtext_in_grid(2, 0, "合格");
    gui_drawtext_in_grid(0, 2, "总数");
    gui_drawtext_in_grid(1, 2, "合格数");
    gui_drawtext_in_grid(2, 2, "日期");
    gui_drawtext_in_grid(0, 4, "CSQ");
    gui_drawtext_in_grid(1, 4, "ID");
    gui_drawtext_in_grid(2, 4, "状态");
}
void start_gui_task(void)
{
    if (fb_open() != 0)
        return;

    if (!ls1x_dc_started())
        return;

    init_simple_gui_env(); /* gui enviroment */
    create_main_objects(); /* main objects */
    grid_content_init();
    start_gui_monitor_task(); /* start button click event task */
    start_flush_task();
}

int start_flush_task(void)
{
    rt_thread_t new_gui_thread = rt_thread_create("flushthread", rt_flush_task, NULL, GUI_STK_SIZE, GUI_TASK_PRIO, GUI_TASK_SLICE);
    if (new_gui_thread == NULL)
    {
        rt_kprintf("create gui thread failed.\n");
        return -1;
    }
    rt_thread_startup(new_gui_thread);
    rt_kprintf("create gui thread succeed.\n");
    return 0;
}
void rt_flush_task(void *arg)
{
    rt_err_t ret;
    rt_mq_buf rbuf;
    while (1)
    {
        ret = rt_mq_recv(output_datas_mq, &rbuf.val_arr, 20, RT_WAITING_FOREVER);
        if (ret == RT_EOK)
        {
            switch (rbuf.val_arr[0])
            {
            case 1:
                draw_texts_in_grids(&rbuf);//刷新单次和总和数据
                break;
            case 2:
                draw_text_in_grid(1, 1, &rbuf.val_arr[1]);//刷新阈值
                break;
            case 3:
                draw_rtc_time_in_grid(2, 3, &rbuf);//刷新时间
                break;
            case 4:
                draw_product_info(&rbuf);//刷新总和数据
                break;
            case 5:
                draw_text_in_grid(0, 5, &rbuf.val_arr[1]);//刷新信号强度CSQ
                break;
            case 6:
                draw_text_in_grid(1, 5, &rbuf.val_arr[1]);//刷新发送成功ID
                gui_drawtext_in_grid(2, 5, "OK");//刷新状态码
                break;
            default:
                break;
            }
        }
    }
}

inline void draw_text_in_grid(int row, int col, unsigned int *val)
{
    char info_buf[8];
    rt_sprintf(info_buf, " %i", *val);
    gui_drawtext_in_grid(row, col, info_buf);
}
inline void draw_text_in_grid_nb(int row, int col, unsigned int *val, char *buf)
{
    rt_sprintf(buf, " %i", *val);
    gui_drawtext_in_grid(row, col, buf);
}
inline void draw_texts_in_grids(rt_mq_buf *buf)
{
    char info_buf[8];
    draw_text_in_grid_nb(0, 1, &buf->val_arr[1], info_buf);
    draw_text_in_grid_nb(2, 1, &buf->val_arr[2], info_buf);
    draw_text_in_grid_nb(0, 3, &buf->val_arr[3], info_buf);
    draw_text_in_grid_nb(1, 3, &buf->val_arr[4], info_buf);
}
void draw_rtc_time_in_grid(int row, int col, rt_mq_buf *buf)
{
    char info_buf[12];
    rt_sprintf(info_buf, "%i.%i.%i", buf->val_arr[1], buf->val_arr[2], buf->val_arr[3]);
    gui_drawtext_in_grid(row, col, info_buf);
}
void draw_product_info(rt_mq_buf *buf)
{
    char info_buf[8];
    draw_text_in_grid_nb(0, 3, &buf->val_arr[1], info_buf);
    draw_text_in_grid_nb(1, 3, &buf->val_arr[2], info_buf);
}

inline void send_btn_msg(unsigned int code)
{
    rt_mq_buf r;
    r.val_arr[0] = code;
    rt_mq_send(main_mq, r.val_arr, 4);
}
