/*
file:
author: 
date:
 */
#include "bsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls1x_fb.h"

#include "yaffs2/port/ls1x_yaffs.h"
#include "yaffs2/direct/yaffsfs.h"
#include "src/file_wr.h"

#include "app_os_priority.h"
#include "src/datastructure_def.h"

extern rt_mq_t main_mq;
extern rt_mq_t file_write_mq;

void send_file_msg(rt_mq_lbuf *buf);
void file_wr_task(void *arg);
int start_file_wr_task(void)
{
    rt_thread_t file_wr_thread = rt_thread_create("filewritethread", file_wr_task, NULL, I2C0_STK_SIZE, 18, 10);
    if (file_wr_thread == NULL)
    {
        rt_kprintf("create file wr thread failed.\n");
        return -1;
    }
    rt_thread_startup(file_wr_thread);
    rt_kprintf("create file wr thread succeed.\n");
    return 0;
}
void file_wr_task(void *arg)
{
    const char *file_dir = "/ndd/userlog";
    const char *file_name = "/ndd/userlog/logfile";
    rt_mq_lbuf wbuf = {.asc_str = {0}};
    rt_mq_lbuf rbuf; 

    rt_err_t ret;
    if (yaffs_is_running())
    {
        yaffs_DIR *temp_dir = yaffs_opendir(file_dir);
        if (!temp_dir)
        {
            yaffs_mkdir(file_dir, S_IFDIR);
            file_write(file_name, wbuf.asc_str, 24, 0);
        }
        else
            yaffs_closedir(temp_dir);

        ret = file_read(file_name, rbuf.asc_str, 24, 0);
        if (ret < 0)
            rt_kprintf("read file failed.\n");
        else
            send_file_msg(&rbuf);
        while (1)
        {
            rt_kprintf("keep waiting for msg form main thread.\n");
            ret = rt_mq_recv(file_write_mq, &rbuf.val_arr, 24, RT_WAITING_FOREVER);
            if (ret == RT_EOK)
            {
                switch (rbuf.val_arr[0])
                {
                case 1: //
                    rbuf.val_arr[0] = 3;
                    ret = file_write(file_name, rbuf.asc_str, 12, 0);
                    if (ret >= 0)
                        rt_kprintf("amount and qualified amount num write successfully.\n");
                    break;
                case 2: //
                    ret = file_write(file_name, &rbuf.asc_str[4], 12, 12);
                    if (ret >= 0)
                        rt_kprintf("date write successfully.\n");
                    break;
                case 3:
                    break;
                default:
                    rt_kprintf("unexcepted condition.\n");
                    break;
                }
            }
        }
    }
    else
        rt_kprintf("yaffs filesystem error.\n");
}

int file_write(const char *file_path, char *wbuf, unsigned int nbyte, unsigned int offset)
{
    int fd;
    fd = yaffs_open(file_path, 0100 | O_RDWR, 0777);
    if (fd >= 0)
    {
        if (offset)
            yaffs_pwrite(fd, wbuf, nbyte, offset);
        else
            yaffs_write(fd, wbuf, nbyte);
        yaffs_flush(fd);
        yaffs_close(fd);
        return 0;
    }
    else
    {
        rt_kprintf("Write file failed.\n");
        return -1;
    }
}
int file_read(const char *file_path, char *rbuf, unsigned int nbyte, unsigned int offset)
{
    int fd = yaffs_open(file_path, 0100 | O_RDWR, 0777);
    if (fd >= 0)
    {
        if (offset)
            yaffs_pread(fd, rbuf, nbyte, offset);
        else
            yaffs_read(fd, rbuf, nbyte);
        yaffs_flush(fd);
        yaffs_close(fd);
        return 0;
    }
    else
    {
        rt_kprintf("Write file failed.\n");
        return -1;
    }
}
void send_file_msg(rt_mq_lbuf *buf)
{
    if (buf->val_arr[0] == 0)//文件内容无效
    {
        buf->val_arr[0] = 2;
        rt_mq_send(main_mq, buf->val_arr, 4);
    }
    else
    {
        buf->val_arr[0] = 3;
        rt_mq_send(main_mq, buf->val_arr, 24);
    }
}
