#ifndef _CommonFileOption_H
#define _CommonFileOption_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//打开一个文件
extern FILE * OpenFile(const char *filename, const char *style, int maxsize = 0/*KB*/);

//关闭文件，把文件指针置空
extern void CloseFile(FILE * &fp);

//获得与格林威治时间的时差
//extern time_t TimeOffset();

//判断文件类型
extern int FileType(const char *file);

//判断是否是普通文件
extern int IsFile(const char *filename);

//判断是否是一个目录
extern int IsDir(const char *directory);

//把字符串写入指定的文件，当文件达到规定的最大长度时，对文件进行备份
extern int WriteStrToFile(const char *filename, const char * text,
	int maxsize = 128,	//文件的最大长度，单位是KB
	int writetime = 1	//是否输出记日志的时间
	);

// 初始化受保护的文件系统及目录信息
extern void InitProtectedFS();
extern bool IsFileLocked(const char * strFile);

#endif // _CommonFileOption_H
