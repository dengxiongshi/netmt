#ifndef _CommonFileOption_H
#define _CommonFileOption_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//��һ���ļ�
extern FILE * OpenFile(const char *filename, const char *style, int maxsize = 0/*KB*/);

//�ر��ļ������ļ�ָ���ÿ�
extern void CloseFile(FILE * &fp);

//������������ʱ���ʱ��
//extern time_t TimeOffset();

//�ж��ļ�����
extern int FileType(const char *file);

//�ж��Ƿ�����ͨ�ļ�
extern int IsFile(const char *filename);

//�ж��Ƿ���һ��Ŀ¼
extern int IsDir(const char *directory);

//���ַ���д��ָ�����ļ������ļ��ﵽ�涨����󳤶�ʱ�����ļ����б���
extern int WriteStrToFile(const char *filename, const char * text,
	int maxsize = 128,	//�ļ�����󳤶ȣ���λ��KB
	int writetime = 1	//�Ƿ��������־��ʱ��
	);

// ��ʼ���ܱ������ļ�ϵͳ��Ŀ¼��Ϣ
extern void InitProtectedFS();
extern bool IsFileLocked(const char * strFile);

#endif // _CommonFileOption_H
