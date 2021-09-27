#ifndef _ConfigFileOptionClass_H
#define _ConfigFileOptionClass_H

#include <stdio.h>
// #include <stdlib.h>

#include "inifile.h"
#include "initdchainmgr.h"

#define MAX_ENUM_STR_VALUE_LEN 256

///////////////////////////////////////////////////////////
//注释链表结构/////////////////////////////////////////////////
class CComment {

public:
    CComment();
    CComment(const char *comment);
    ~CComment();

    void Push(const char *comment);
    void Push(CComment * comment);
    bool find(const char * comment);
    void Print(FILE *fp=stdout, const char *head = "");

private:
    CComment* m_pNext;
    char*     m_strComment;
};

///////////////////////////////////////////////////////////
//配置记录结构///////////////////////////////////////////////////
class CRecord
{
public:
    char    m_strName[256];
    char    m_strValue[256];
    CComment    *m_chainComment;

public:
    CRecord();
    CRecord(const char *name, const char *value);
    ~CRecord();

    int SetName(const char * name);
    int SetValue(const char * name, const char *value = NULL);
    void SetComment(CComment * &comment);
    void SetComment(const char * comment);
    void Print(FILE *fp = stdout, bool display_config_value = false);
    int operator == ( CRecord & right) const;
    int operator == (const char *name) const;
    int operator >  ( CRecord & right) const;
    int operator >= ( CRecord &) const {return 1;}
    int operator != ( CRecord &) const {return 1;}
    int operator <  ( CRecord &) const {return 1;}
    int operator <= ( CRecord &) const {return 1;}
};

///////////////////////////////////////////////////////////
//枚举型值链表///////////////////////////////////////////////////
class CEnumValue
{
public:
    CEnumValue();
    CEnumValue(int index, const char *value);
    ~CEnumValue();

    void SetValue(int index, const char *value);

    int operator == ( CEnumValue & right) const;
    int operator == (const char *value) const;
    int operator == ( int index) const;
    int operator >  ( CEnumValue & right) const;
    int operator >= ( CEnumValue &) const {return 1;}
    int operator != ( CEnumValue &) const {return 1;}
    int operator <  ( CEnumValue &) const {return 1;}
    int operator <= ( CEnumValue &) const {return 1;}
    void Print(FILE *fp, const char *format);

public:
    int m_nIndex;
    char m_strValue[256];

};

///////////////////////////////////////////////////////////
//配置类结构////////////////////////////////////////////////////
class CSection
{
public:
    char m_strSection[256];
    CTDChainMgr<CRecord *,1,CtEM_REJECT,1>      *m_dcmRecord;
    CTDChainMgr<CEnumValue *,0,CtEM_REJECT,1>   *m_dcmEnumValue;
    int m_nEnumValueMaxWidth;
    int m_nEnumValueColumn;
    CRecord     *m_cRecord;
    CComment    *m_chainComment;

public:
    CSection();
    CSection(const char *section, int column);
    ~CSection();

    CEnumValue * SearchEnumValue(int index);
    char * GetValue(int index);
    int SetEnumValue(const char *section, int index, const char *value);
    int SetEnumValue(const char *section, const char *value);
    int EnumValueNum(int column);

    CRecord * SearchRecord(const char *name);
    char * GetValue(const char *name);
    int SetSection(const char * section);
    int SetValue(const char *section, const char *name, const char *value, CComment *&comment);
    void SetComment(const char * comment);
    void SetComment(CComment * &comment);
    int operator == ( CSection & right) const;
    int operator == (const char *section) const;
    int operator > ( CSection & right) const;
    int operator >= ( CSection &) const {return 1;}
    int operator != ( CSection &) const {return 1;}
    int operator <  ( CSection &) const {return 1;}
    int operator <= ( CSection &) const {return 1;}
    void Print(FILE *fp = stdout, bool display_config_value = false);
};

///////////////////////////////////////////////////////////
//配置操作类////////////////////////////////////////////////////
//class CConfOpt : public ZcBase_T
class CConfOpt
{
private:
    char m_strFileName[256];
    FILE * m_fpFile;
    CTDChainMgr<CSection *,1,CtEM_REJECT,1> * m_dcmSection;
    CSection    *m_cSection;
    CComment    *m_chainComment;

    char * FileName(const char * filename);
    CSection * SearchSection(const char *section);
    void AddSection(const char *section, int column, const char *comment);
    void SetDefaultPrompt(const char * name, const char *defv);

    int m_bConfigChange;
    bool m_bDisplayConfigValue;
    bool m_bDisplayDefaultPrompt;
    int m_bItemExist;

public:
    CConfOpt(int display_config_value = 0, int display_default_prompt = 0);
    CConfOpt(const char *filename, int display_config_value = 0, int display_default_prompt = 0);
    ~CConfOpt();
    void SetWriteFlag(int want_write);
    void FreeMemory();

    //输出配置信息到指定文件
    void WriteToFile(const char *filename = NULL);

    //从指定文件读取配置信息
    void ReadFromFile(const char *filename = NULL);

    int DeleteSection(const char *section);
    int CleanSection(const char *section);
    int CleanEnum(const char *section);

    //设置指定项目的配置值
    void SetEnumValue(const char *section, const char *value, const char *comment);
    void SetEnumValue(const char *section, const char *value, int column, const char *comment);
    void SetValue(const char *section, const char *name, const int value, const char *comment);
    void SetValue(const char *section, const char *name, const float value, const char *comment);
    void SetValue(const char *section, const char *name, const unsigned char value, const char *comment);
    void SetValue(const char *section, const char *name, const char * value, const char *comment);

    //得到枚举型配置项目的个数
    int GetCount(const char *section, const char *comment = NULL);
    int GetCount(const char *section, int column, const char *comment = NULL);

    //得到枚举型配置项目的第N个值
    int GetValue(const char *section, int index, char *value, const char *comment = NULL);

    //得到指定项目的配置值
    int GetValue(const char *section, const char *name, const int defv, int &value, const char *comment = NULL);
    int GetValue(const char *section, const char *name, const float defv, float &value, const char *comment = NULL);
    int GetValue(const char *section, const char *name, const unsigned char defv, unsigned char &value, const char *comment = NULL);
    int GetValue(const char *section, const char *name, const char * defv, char * value, const char *comment = NULL);

    //判断配置项是否存在
    int IsExist(const char *section, const char *name);

private:
    int GetValue(const char *section, const char *name, char * value, const char *comment);
};

// #ifdef __cplusplus
// extern "C" {
// #endif

// extern int CGetValueInt(char* szfilename,const char *section, const char *name, int defv, int *value, const char *comment = NULL);
// extern int CGetValueString(char* szfilename,const char *section, const char *name, const char * defv, char * value, const char *comment = NULL);
// extern int CGetValueArr(char* szfilename,const char *section, char outarr[100][200]);
// extern int CGetEnumValueArr(char* szfilename,const char *section, char** poutput, int max_len);
// extern int CSetEnumValue(char* szfilename,const char *section, const char *value );


// #ifdef __cplusplus
// }
// #endif

#endif // _ConfigFileOptionClass_H




