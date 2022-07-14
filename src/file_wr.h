/*
 * file_wr.h
 *
 * created: 2022/7/9
 *  author:teamreach 
 */

#ifndef _FILE_WR_H
#define _FILE_WR_H


int file_write(const char* file_path,char* wbuf,unsigned int nbyte ,unsigned int offset);
int file_read(const char* file_path,char* rbuf,unsigned int nbyte ,unsigned int offset);

#endif // _FILE_WR_H

