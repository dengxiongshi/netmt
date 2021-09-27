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

//按照第一次出现的分隔符把字符串分成两部分，源串中允许包含两个以上的分隔符
extern int SeperateByFirstDelimiter(const char *src, char *first, char *second, const char delimiter = '=');

//按照第一次出现的分隔符把字符串分成两部分，如果源串中包含两个以上的分隔符，则分割失败
extern int SeperateByUniqueDelimiter(const char *src, char *first, char *second, const char delimiter = '=');

//返回第一个有效字符
extern char FirstValidChar(char *string);

//按照给定的分隔符把字符分隔成最多num个子串
extern int Seperate(const char *string, char *part[], int &num, char delimiter =' ');

//去掉开头的空格和制表符
extern void LeftTrim(char *string);

//去掉结尾的空格和制表符及回车
extern void RightTrim(char *string);

//把重复的空格和制表符去掉
extern void Format(const char * string, char *result);
extern void Format(char * string);

//把所有的字母转成大写
extern char * ToUpper(char *string);
extern const char * ToUpper(const char *str, char * result);

//把所有的字母转成小写
extern char * ToLower(char *string);
extern const char * ToLower(const char *str, char * result);

//取环境变量的值
extern char * GetEnv(char * dst, const char *envname, const char * defval);

//字符串比较, sensitive指示是否大小写敏感，相等返回零
extern int StrCmp(const char *st1, const char *str2, const int sensitive = 1);

//比较字符串的后N位, 只返回相等和不等两个结果
extern int StrCmpTail(const char * str1, const char * str2, const int len);

//连接字符串，separator是两个字符串的连接串
extern void StrCat(char *str1, const char *str2, const char *separator = NULL);

#endif //__CommonStringOption_H
