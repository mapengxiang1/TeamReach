/*
file:datastructure_def.h
author:teamreach 
date:2022.7.6
 */

#ifndef _DATASTRUCTURE_DEF_H
#define _DATASTRUCTURE_DEF_H
#include "ls1x_rtc.h"

typedef struct tm rtc_obj;
typedef struct
{
    unsigned int year;
    unsigned int mon;
    unsigned int day;
} date_info;

typedef struct
{
    unsigned int v_measure;
    unsigned int v_threshold;
    int qualified;
} res_paras;
typedef struct
{
    unsigned int total;
    unsigned int qualified_amount;//合格品总数
    unsigned int year;
    unsigned int mon;
    unsigned int day;
    unsigned int hour;
    unsigned int min;
} product_info;
typedef struct
{
    unsigned int csq;
    unsigned int msg_id;//JSONString ID,用以区别不同消息
    int stat_code;
} tr_stat;
typedef union
{
    unsigned int val_arr[5];
    char asc_str[20];
} rt_mq_buf;
typedef union
{
    unsigned int val_arr[6];
    char asc_str[24];
} rt_mq_lbuf;
#endif // _DATASTRUCTURE_DEF_H
