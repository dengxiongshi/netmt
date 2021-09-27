#ifndef __CommonStringOption_H
#define __CommonStringOption_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef IS_STR_VALID
#undef IS_STR_VALID
#endif
#define IS_STR_VALID(string) ((string)&&strcmp((string), ""))

#ifdef IS_DELIMITER
#undef IS_DELIMITER
#endif
#define IS_DELIMITER(c)	( (c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')

#ifdef DeletePointer
#undef DeletePointer
#endif
#define DeletePointer(pointer) \
{\
	if( pointer ) \
	{ \
		delete pointer; \
		pointer = NULL; \
	} \
}

//���յ�һ�γ��ֵķָ������ַ����ֳ������֣�Դ������������������ϵķָ���
extern int SeperateByFirstDelimiter(const char *src, char *first, char *second, const char delimiter = '=');

//���յ�һ�γ��ֵķָ������ַ����ֳ������֣����Դ���а����������ϵķָ�������ָ�ʧ��
extern int SeperateByUniqueDelimiter(const char *src, char *first, char *second, const char delimiter = '=');

//���ص�һ����Ч�ַ�
extern char FirstValidChar(char *string);

//���ո����ķָ������ַ��ָ������num���Ӵ�
extern int Seperate(const char *string, char *part[], int &num, char delimiter =' ');

//ȥ����ͷ�Ŀո���Ʊ��
extern void LeftTrim(char *string);

//ȥ����β�Ŀո���Ʊ�����س�
extern void RightTrim(char *string);

//���ظ��Ŀո���Ʊ��ȥ��
extern void Format(const char * string, char *result);
extern void Format(char * string);

//�����е���ĸת�ɴ�д
extern char * ToUpper(char *string);
extern const char * ToUpper(const char *str, char * result);

//�����е���ĸת��Сд
extern char * ToLower(char *string);
extern const char * ToLower(const char *str, char * result);

//ȡ����������ֵ
extern char * GetEnv(char * dst, const char *envname, const char * defval);

//�ַ����Ƚ�, sensitiveָʾ�Ƿ��Сд���У���ȷ�����
extern int StrCmp(const char *st1, const char *str2, const int sensitive = 1);

//�Ƚ��ַ����ĺ�Nλ, ֻ������ȺͲ����������
extern int StrCmpTail(const char * str1, const char * str2, const int len);

//�����ַ�����separator�������ַ��������Ӵ�
extern void StrCat(char *str1, const char *str2, const char *separator = NULL);

#endif //__CommonStringOption_H
