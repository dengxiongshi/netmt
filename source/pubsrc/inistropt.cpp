#include "inistropt.h"

#include <ctype.h>

char FirstValidChar(char *string)
{
    if( !string ) return 0;

    char *p = string;
    while( p && IS_DELIMITER(*p) ) p++;
    return *p;
}

void LeftTrim(char *string)
{
    if( !string ) return;

    char *p = string;
    while( IS_DELIMITER(*p) ) p++;
    while( *p ) *(string++) = *(p++);
    *string = 0;
}

void RightTrim(char *string)
{
    if( !string ) return;

    char *p = string+strlen(string)-1;
    while( IS_DELIMITER(*p) ) p--;
    *(p+1) = 0;
}

void Format(const char * string, char *result)
{
    char * p = (char *)string;
    int nFlag = 0;

    if( !IS_STR_VALID(string) || !result ) return;

    while( IS_DELIMITER(*p) ) p++;
    while( *p )
    {
        if( IS_DELIMITER(*p) )
        {
            if( nFlag )
            {
                p++;
                continue;
            }
            nFlag = 1;
        }
        else nFlag = 0;
        *(result++) = *(p++);
    }
    while( IS_DELIMITER(*(result-1)) ) result--;
    *result = 0;
}

void Format(char * string)
{
    char * p = string;
    int nFlag = 0;

    if( !string ) return;

    RightTrim(string);
    while( IS_DELIMITER(*p) ) p++;
    while( *p )
    {
        if( IS_DELIMITER(*p) )
        {
            if( nFlag )
            {
                p++;
                continue;
            }
            nFlag = 1;
        }
        else nFlag = 0;
        *(string++) = *(p++);
    }
    *string = 0;
}

char * ToUpper(char *string)
{
    if( !string ) return NULL;
    char * p = string;
    while (*p)
    {
        *p = toupper(*p);
        p++;
    }
    return string;
}

const char * ToUpper(const char *str, char * result)
{
    static char strTemp[256];
    if (NULL == result) result = strTemp;
    *result = 0;

    if (!str) return result;
    int i = 0;
    while (str[i] != 0)
    {
        result[i] = (islower(str[i]))?toupper(str[i]):str[i];
        i++;
    }
    result[i] = 0;
    return result;
}

char * ToLower(char *string)
{
    if( !string ) return NULL;
    char * p = string;
    while( *p )
    {
        *p = tolower(*p);
        p++;
    }
    return string;
}

const char * ToLower(const char *str, char * result)
{
    static char strTemp[256];
    if (NULL == result) result = strTemp;
    *result = 0;

    if (!str) return result;
    int i = 0;
    while (str[i] != 0)
    {
        result[i] = (isupper(str[i]))?tolower(str[i]):str[i];
        i++;
    }
    result[i] = 0;
    return result;
}

int SeperateByFirstDelimiter(const char *src, char *first, char *second, const char delimiter)
{
    char *pFirst;

    if( !IS_STR_VALID(src) || !first || !second ) return 0;
        if (delimiter == 0) return 0;

    //清空返回串
    *first = *second = 0;

    pFirst = (char *)strchr(src, delimiter);
    if( !pFirst )
    { //传入串中不包含指定的分隔符
        strcpy( first, src);
        Format(first);
        *second = 0;
        return 1;
    }

    //考贝到返回串，并格式化返回串
    strncpy( first, src, pFirst-src);
    *(first+(pFirst-src)) = 0;
    strcpy( second, pFirst+1);
    Format(first);
    Format(second);
    return 1;
}

int SeperateByUniqueDelimiter(const char *src, char *first, char *second, const char delimiter)
{
    char *pFirst, *pLast;

    if( !IS_STR_VALID(src) || !first || !second ) return 0;
        if (delimiter == 0) return 0;

    //清空返回串
    *first = *second = 0;

    pFirst = (char *)strchr(src, delimiter);
    if( !pFirst )
    { //传入串中不包含指定的分隔符
        strcpy( first, src);
        Format(first);
        *second = 0;
        return 1;
    }

    //判断是否存在两个以上指定的分隔符
    pLast = (char *)strrchr(src, delimiter);
    if( pLast != pFirst ) return 0;

    //考贝到返回串，并格式化返回串
    strncpy( first, src, pFirst-src);
    *(first+(pFirst-src)) = 0;
    strcpy( second, pFirst+1);
    Format(first);
    Format(second);
    return 1;
}

int Seperate(const char *string, char *part[], int &num, char delimiter)
{
    char *pHead = (char *)string, *pTail;
    int i = 0;

    while( *pHead == delimiter ) pHead ++;
    while( pHead && i<num)
    {
        
        pTail = strchr(pHead, delimiter);
        if( pTail )
        {
            strncpy( part[i], pHead, pTail-pHead);
            part[i++][pTail-pHead] = 0;
            pHead = pTail+1;
            while( *pHead == delimiter ) pHead ++;
        }
        else
        {
            strcpy( part[i++], pHead);
            pHead = NULL;
        }
    }
    num = i;
    return i;
}

char * GetEnv(char * dst, const char *envname, const char * defval)
{
    char *pTemp;

    if( !dst || !IS_STR_VALID(envname) ) return NULL;

    *dst = 0;

    pTemp = getenv(envname);
    if( !pTemp )
    {
        if( IS_STR_VALID(defval) ) memcpy((void *)dst, (void *)defval, strlen(defval));
    }
    else memcpy((void *)dst, (void *)pTemp, strlen(pTemp));
    return dst;
}

int StrCmp(const char *str1, const char *str2, const int sensitive)
{
    if( !IS_STR_VALID(str1) && !IS_STR_VALID(str2) ) return 0;
    else if( !IS_STR_VALID(str1) ) return -1;
    else if( !IS_STR_VALID(str2) ) return 1;
    else if( sensitive ) return strcmp(str1, str2);
    else
    {   //大小写不敏感，转换成大写后比较
        char c1 = toupper(*str1);
        char c2 = toupper(*str2);
        if( c1 > c2 ) return 1;
        else if( c1 < c2 ) return -1;
        else return StrCmp(str1+1, str2+1, sensitive);
    }
}

int StrCmpTail(const char * str1, const char * str2, const int len)
{
    if( !IS_STR_VALID(str1) || !IS_STR_VALID(str2) ) return 0;
    int nLen1 = strlen(str1);
    int nLen2 = strlen(str2);
    if (nLen1 <= len || nLen2 <= len) return 0;
    return (0 == strcmp(&str1[nLen1-len], &str2[nLen2-len]));
}

void StrCat(char *str1, const char *str2, const char *separator)
{
    if( !str1 || !IS_STR_VALID(str2) ) return;
    if( IS_STR_VALID(separator) && *str1 ) strcat(str1, separator);
    strcat(str1, str2);
}

