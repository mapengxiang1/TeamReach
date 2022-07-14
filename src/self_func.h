/*
file:self_dunc.h
author:teamreach 
date:2022.7.6
 */

#ifndef _SELF_FUNC_H
#define _SELF_FUNC_H

#include "datastructure_def.h"

void init_rtc_time(rtc_obj *, product_info *);

void input_interrupt_isr(int vector, void *param);
void input_interrupt_isr2(int vector, void *param);
void init_datastructure(res_paras *, product_info *, tr_stat *);

void set_v_measure(res_paras *, unsigned short int);
void set_qualified(res_paras *);
void set_product_info(rt_mq_buf *, product_info *);
void set_csq(rt_mq_buf *, tr_stat *);
void set_date_time(rt_mq_buf *, product_info *);
void set_output(res_paras *);
void set_msg_id(tr_stat *);

void add_porduct(product_info *, res_paras *);

void send_data_msg(res_paras *, product_info *);
void send_v_threshold_msg(res_paras *);
void send_rtc_msg(product_info *);
void send_product_info_msg(product_info *);
void send_csq(tr_stat *);
void send_nb_msg(res_paras *, tr_stat *);
void send_msg_id_msg(tr_stat *);
void send_stat_code_msg(tr_stat *);
void send_info_to_file_msg(product_info *);
void send_date_to_file_msg(product_info *);

extern int start_file_wr_task(void);
extern int start_nb_task(void);
#endif // _SELF_FUNC_H
