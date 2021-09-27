#include "iniconfopt.h"
#include "inistropt.h"
#include "inifileopt.h"

#define LINE_WIDTH 60

bool l_bDisplayConfigValue = 0;

///////////////////////////////////////////////////////////
//注释结构/////////////////////////////////////////////////////
CComment::CComment()
{
    m_pNext = NULL;
    m_strComment = NULL;
}

CComment::CComment(const char *comment)
{
    m_pNext = NULL;
    m_strComment = NULL;
    if (comment)
    {
        int nLen;
        nLen = strlen(comment) + 2;

        m_strComment = new char[nLen];
        memset(m_strComment, 0, nLen);
        if (*comment != '#')
            strcpy(m_strComment, "#");
        strcat(m_strComment, comment);
    }
}

CComment::~CComment()
{
    if (m_strComment)
        delete[] m_strComment;
    if (m_pNext)
        delete m_pNext;
}

void CComment::Push(const char *comment)
{
    if (find(comment))
        return;
    if (m_pNext)
        m_pNext->Push(comment);
    else
        m_pNext = new CComment(comment);
}

void CComment::Push(CComment *comment)
{
    if (find(comment->m_strComment))
        return;
    if (m_pNext)
        m_pNext->Push(comment);
    else
        m_pNext = comment;
}

bool CComment::find(const char *comment)
{
    CComment *p = this;
    while (p != NULL)
    {
        if (0 == strcmp(m_strComment, comment))
            return true;
        p = p->m_pNext;
    }
    return false;
}

void CComment::Print(FILE *fp, const char *head)
{
    fprintf(fp, "%s%s\n", head, m_strComment);
    if (NULL != m_pNext)
        m_pNext->Print(fp, (head != NULL && *head != 0) ? "\t" : "");
}

///////////////////////////////////////////////////////////
//配置记录结构///////////////////////////////////////////////////
CRecord::CRecord()
{
    memset(m_strName, 0, sizeof(m_strName));
    memset(m_strValue, 0, sizeof(m_strValue));
    m_chainComment = NULL;
}

CRecord::CRecord(const char *name, const char *value)
{
    memset(m_strName, 0, sizeof(m_strName));
    memset(m_strValue, 0, sizeof(m_strValue));
    if (SetName(name))
        SetValue(name, value);
    m_chainComment = NULL;
}

CRecord::~CRecord()
{
    DeletePointer(m_chainComment);
}

int CRecord::SetName(const char *name)
{
    if (!IS_STR_VALID(name))
        return 0;

    strcpy(m_strName, name);
    return 1;
}

int CRecord::SetValue(const char *name, const char *value)
{
    if (!IS_STR_VALID(name))
        return 0;

    if (strcmp(m_strName, name))
        return 0;

    if (!IS_STR_VALID(value) || !strcmp(value, "DEFAULT"))
        memset(m_strValue, 0, sizeof(m_strValue));
    else
        strcpy(m_strValue, value);

    return 1;
}

void CRecord::SetComment(const char *comment)
{
    if (NULL == comment)
        return;

    if (!m_chainComment)
        m_chainComment = new CComment(comment);
    else
    {
        if (m_chainComment->find(comment))
            return;
        CComment *p = new CComment(comment);
        m_chainComment->Push(p);
    }
}

void CRecord::SetComment(CComment *&comment)
{
    if (NULL == comment)
        return;
    if (!m_chainComment)
        m_chainComment = comment;
    else
        m_chainComment->Push(comment);
    comment = NULL;
}

void CRecord::Print(FILE *fp, bool display_config_value)
{
    if (fp == NULL)
        fp = stdout;
    if (m_chainComment)
        m_chainComment->Print(fp, "\n\t");
    fprintf(fp, "\t%s = %s\n", m_strName,
            IS_STR_VALID(m_strValue) ? m_strValue : display_config_value ? ""
                                                                         : "DEFAULT");
}

int CRecord::operator==(CRecord &right) const
{
    return !strcmp(m_strName, right.m_strName);
}

int CRecord::operator==(const char *name) const
{
    return !strcmp(m_strName, name);
}

int CRecord::operator>(CRecord &right) const
{
    return (strcmp(m_strName, right.m_strName) > 0);
}

///////////////////////////////////////////////////////////
//枚举型值链表///////////////////////////////////////////////////
CEnumValue::CEnumValue() : m_nIndex(0)
{
    memset(m_strValue, 0, sizeof(m_strValue));
}

CEnumValue::CEnumValue(int index, const char *value) : m_nIndex(index)
{
    strcpy(m_strValue, value);
}

CEnumValue::~CEnumValue()
{
}

void CEnumValue::SetValue(int index, const char *value)
{
    if (m_nIndex == index)
        strcpy(m_strValue, value);
}

void CEnumValue::Print(FILE *fp, const char *format)
{
    if (fp == NULL)
        fp = stdout;
    fprintf(fp, format, m_strValue);
}

int CEnumValue::operator==(CEnumValue &right) const
{
    return !strcmp(m_strValue, right.m_strValue);
}

int CEnumValue::operator==(const char *value) const
{
    return !strcmp(m_strValue, value);
}

int CEnumValue::operator==(int index) const
{
    return m_nIndex == index;
}

int CEnumValue::operator>(CEnumValue &right) const
{
    return (strcmp(m_strValue, right.m_strValue) > 0);
}

///////////////////////////////////////////////////////////
//配置类结构////////////////////////////////////////////////////
CSection::CSection()
{
    memset(m_strSection, 0, sizeof(m_strSection));
    m_dcmRecord = new CTDChainMgr<CRecord *, 1, CtEM_REJECT, 1>;
    m_dcmEnumValue = new CTDChainMgr<CEnumValue *, 0, CtEM_REJECT, 1>;
    m_chainComment = NULL;
    m_cRecord = NULL;
    m_nEnumValueMaxWidth = 10;
    m_nEnumValueColumn = 0;
}

CSection::CSection(const char *section, int column)
{
    strcpy(m_strSection, section);
    m_dcmRecord = new CTDChainMgr<CRecord *, 1, CtEM_REJECT, 1>;
    m_dcmEnumValue = new CTDChainMgr<CEnumValue *, 0, CtEM_REJECT, 1>;
    m_chainComment = NULL;
    m_cRecord = NULL;
    m_nEnumValueMaxWidth = 10;
    m_nEnumValueColumn = column;
}

CSection::~CSection()
{
    DeletePointer(m_dcmRecord);
    DeletePointer(m_dcmEnumValue);
    DeletePointer(m_cRecord);
    DeletePointer(m_chainComment);
}

int CSection::EnumValueNum(int column)
{
    m_nEnumValueColumn = column;
    return m_dcmEnumValue->m_nNodeNum;
}

CEnumValue *CSection::SearchEnumValue(int index)
{
    CEnumValue *cEnumValue;
    if (index >= m_dcmEnumValue->m_nNodeNum)
        return NULL;

    for (cEnumValue = m_dcmEnumValue->Begin(); cEnumValue; cEnumValue = m_dcmEnumValue->Next())
    {
        if (*cEnumValue == index)
            return cEnumValue;
    }
    return NULL;
}

char *CSection::GetValue(int index)
{
    CEnumValue *cEnumValue = SearchEnumValue(index);

    if (cEnumValue)
        return cEnumValue->m_strValue;
    else
        return NULL;
}

int CSection::SetEnumValue(const char *section, const char *value)
{
    return SetEnumValue(section, m_dcmEnumValue->m_nNodeNum, value);
}

int CSection::SetEnumValue(const char *section, int index, const char *value)
{
    if (strcmp(m_strSection, section))
        return 0;

    CEnumValue *cEnumValue = SearchEnumValue(index);

    if (m_nEnumValueMaxWidth < (int)strlen(value))
        m_nEnumValueMaxWidth = strlen(value);

    if (cEnumValue)
        cEnumValue->SetValue(index, value);
    else
    {
        cEnumValue = new CEnumValue(index, value);
        m_dcmEnumValue->Push(cEnumValue);
    }
    return 1;
}

CRecord *CSection::SearchRecord(const char *name)
{
    CRecord *cRecord;
    if (!IS_STR_VALID(name))
        return NULL;

    for (cRecord = m_dcmRecord->Begin(); cRecord; cRecord = m_dcmRecord->Next())
    {
        if (*cRecord == name)
            return cRecord;
    }
    return NULL;
}

char *CSection::GetValue(const char *name)
{
    CRecord *cRecord = SearchRecord(name);

    if (cRecord)
        return cRecord->m_strValue;
    else
        return NULL;
}

int CSection::SetSection(const char *section)
{
    if (!IS_STR_VALID(section))
        return 0;
    strcpy(m_strSection, section);
    return 1;
}

int CSection::SetValue(const char *section, const char *name, const char *value, CComment *&comment)
{
    if (strcmp(m_strSection, section))
        return 0;

    CRecord *cRecord = SearchRecord(name);

    if (cRecord)
        cRecord->SetValue(name, value);
    else
    {
        cRecord = new CRecord(name, value);
        m_dcmRecord->Push(cRecord);
    }
    cRecord->SetComment(comment);
    return 1;
}

void CSection::SetComment(const char *comment)
{
    if (NULL == comment)
        return;

    if (!m_chainComment)
        m_chainComment = new CComment(comment);
    else
    {
        if (m_chainComment->find(comment))
            return;
        CComment *p = new CComment(comment);
        m_chainComment->Push(p);
    }
}

void CSection::SetComment(CComment *&comment)
{
    if (!m_chainComment)
        m_chainComment = comment;
    else
        m_chainComment->Push(comment);
    comment = NULL;
}

int CSection::operator==(CSection &right) const
{
    return !strcmp(m_strSection, right.m_strSection);
}

int CSection::operator==(const char *section) const
{
    return !strcmp(m_strSection, section);
}

int CSection::operator>(CSection &right) const
{
    return (strcmp(m_strSection, right.m_strSection) > 0);
}

void CSection::Print(FILE *fp, bool display_config_value)
{
    CRecord *cRecord;
    CEnumValue *cEnumValue;
    char strFormat[20];
    int nNum, nColumn;

    if (fp == NULL)
        fp = stdout;
    if (m_chainComment)
        m_chainComment->Print(fp);
    fprintf(fp, "[%s]\n", m_strSection);

    nNum = 0;
    for (cRecord = m_dcmRecord->Begin(); cRecord; cRecord = m_dcmRecord->Next())
    {
        cRecord->Print(fp, display_config_value);
        nNum++;
    }
    if (nNum)
        fprintf(fp, "\n");

    nNum = 0;
    nColumn = (m_nEnumValueColumn > 0) ? m_nEnumValueColumn : LINE_WIDTH / m_nEnumValueMaxWidth;
    sprintf(strFormat, "%%-%ds", m_nEnumValueMaxWidth + 1);
    for (cEnumValue = m_dcmEnumValue->Begin(); cEnumValue; cEnumValue = m_dcmEnumValue->Next())
    {
        //if( nNum == 0 ) fprintf(fp, "\t");
        //else if( nNum%nColumn == 0 ) fprintf(fp, "\n\t");
        cEnumValue->Print(fp, strFormat);
        fprintf(fp, "\n");
        nNum++;
    }
    //if( nNum%nColumn) fprintf( fp, "\n\n");
    //else if( nNum ) fprintf(fp, "\n");
    fprintf(fp, "\n");
}

///////////////////////////////////////////////////////////
//配置操作类////////////////////////////////////////////////////
CConfOpt::CConfOpt(int display_config_value, int display_default_prompt)
{
    m_bConfigChange = 0;
    m_bDisplayConfigValue = display_config_value;
    m_bDisplayDefaultPrompt = display_default_prompt;
    m_chainComment = 0;
    memset(m_strFileName, 0, sizeof(m_strFileName));
    m_fpFile = NULL;
    m_dcmSection = new CTDChainMgr<CSection *, 1, CtEM_REJECT, 1>;
    m_cSection = NULL;
    m_bItemExist = 0;
}

CConfOpt::CConfOpt(const char *filename, int display_config_value, int display_default_prompt)
{
    m_bConfigChange = 0;
    m_bDisplayConfigValue = display_config_value;
    m_bDisplayDefaultPrompt = display_default_prompt;
    m_chainComment = 0;
    if (IS_STR_VALID(filename))
        strcpy(m_strFileName, filename);
    else
        memset(m_strFileName, 0, sizeof(m_strFileName));
    m_fpFile = NULL;
    m_dcmSection = new CTDChainMgr<CSection *, 1, CtEM_REJECT, 1>;
    m_cSection = NULL;
    m_bItemExist = 0;

    if (IS_STR_VALID(m_strFileName))
    {
        ReadFromFile();
    }
}

CConfOpt::~CConfOpt()
{
    // WriteToFile();
    DeletePointer(m_dcmSection);
    m_cSection = NULL;
    DeletePointer(m_chainComment);
    CloseFile(m_fpFile);
}

char *CConfOpt::FileName(const char *filename)
{
    if (IS_STR_VALID(filename))
        return (char *)filename;
    else if (IS_STR_VALID(m_strFileName))
        return m_strFileName;
    else
        return NULL;
}

void CConfOpt::FreeMemory()
{
    if (m_dcmSection)
        m_dcmSection->Empty();
    DeletePointer(m_chainComment);
    m_cSection = NULL;
}

CSection *CConfOpt::SearchSection(const char *section)
{
    CSection *cSection;

    for (cSection = m_dcmSection->Begin(); cSection; cSection = m_dcmSection->Next())
    {
        if (!strcmp(cSection->m_strSection, section))
            return cSection;
    }
    return NULL;
}

void CConfOpt::WriteToFile(const char *filename)
{
    char strTemp[256], *pFileName;

    if (!m_bConfigChange)
        return;
    m_bConfigChange = 0;

    if (NULL == (pFileName = FileName(filename)))
        return;

    if (IsFile(pFileName))
    { //如果文件存在，则对文件做备份
        sprintf(strTemp, "cp %s %s.bak", pFileName, pFileName);
        system(strTemp);
    }

    CloseFile(m_fpFile);
    if (NULL == (m_fpFile = OpenFile(pFileName, "w")))
        return;
    CSection *cSection;
    for (cSection = m_dcmSection->Begin(); cSection; cSection = m_dcmSection->Next())
    {
        cSection->Print(m_fpFile, m_bDisplayConfigValue);
    }

    CloseFile(m_fpFile);
}

void CConfOpt::ReadFromFile(const char *filename)
{
    char strBuf[256], strSection[256], strName[256], strValue[256];
    char cFirst, *pTemp;

    FreeMemory();
    m_bConfigChange = 0;

    if (NULL == (pTemp = FileName(filename)))
        return;

    CloseFile(m_fpFile);
    if (NULL == (m_fpFile = OpenFile(pTemp, "r")))
        return;

    memset(strSection, 0, sizeof(strSection));
    memset(strName, 0, sizeof(strName));
    memset(strValue, 0, sizeof(strValue));

    while (fgets(strBuf, 255, m_fpFile))
    {
        cFirst = FirstValidChar(strBuf);
        if (!cFirst)
            continue;
        else if (cFirst == '[')
        {
            pTemp = strrchr(strBuf, ']');
            if (!pTemp)
                printf("there some error at section\n");
            else
            {
                *pTemp = 0;
                pTemp = strchr(strBuf, '[');
                *pTemp = ' ';
                Format(strBuf);
                strcpy(strSection, strBuf);
                if (NULL == (m_cSection = SearchSection(strSection)))
                {
                    m_cSection = new CSection(strBuf, 0);
                    m_dcmSection->Push(m_cSection);
                }

                m_cSection->SetComment(m_chainComment);
            }
        }
        else if (cFirst == '#' || cFirst == ';' || !strcmp(strSection, ""))
        {
            RightTrim(strBuf);
            LeftTrim(strBuf);
            if (!m_chainComment)
                m_chainComment = new CComment(strBuf);
            else
                m_chainComment->Push(strBuf);
        }
        else
        {
            if (NULL == m_cSection)
                continue;

            if (strchr(strBuf, '='))
            {
                if (!SeperateByFirstDelimiter(strBuf, strName, strValue, '='))
                {
                    printf("there some error at source string\n");
                    // exit(0);
                }
                m_cSection->SetValue(strSection, strName, strValue, m_chainComment);
            }
            else
            {
                pTemp = strBuf;
                while (*pTemp)
                {
                    while (IS_DELIMITER(*pTemp))
                        pTemp++;
                    if (!(*pTemp))
                        break;
                    sscanf(pTemp, "%s", strValue);
                    m_cSection->SetEnumValue(strSection, strValue);
                    pTemp += strlen(strValue);
                }
                m_bConfigChange = 1;
            }
        }
    }
    CloseFile(m_fpFile);
}

void CConfOpt::SetEnumValue(const char *section, const char *value, const char *)
{
    SetEnumValue(section, value, 0, NULL);
}

void CConfOpt::SetEnumValue(const char *section, const char *value, int column, const char *)
{
    char strSection[256], strValue[128];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(value))
        return;

    Format(section, strSection);
    sscanf(value, "%s", strValue);
    if (NULL == (m_cSection = SearchSection(strSection)))
    {
        m_cSection = new CSection(strSection, column);
        m_dcmSection->Push(m_cSection);
    }

    m_bConfigChange = 1;
    m_cSection->SetEnumValue(strSection, strValue);
}

void CConfOpt::SetValue(const char *section, const char *name, const int value, const char *comment)
{
    char strTemp[32];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name))
        return;
    sprintf(strTemp, "%d", value);
    SetValue(section, name, strTemp, comment);
}

void CConfOpt::SetValue(const char *section, const char *name, const float value, const char *comment)
{
    char strTemp[32];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name))
        return;
    sprintf(strTemp, "%f", value);
    SetValue(section, name, strTemp, comment);
}

void CConfOpt::SetValue(const char *section, const char *name, const unsigned char value, const char *comment)
{
    char strTemp[32];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name))
        return;
    sprintf(strTemp, "%c", value);
    SetValue(section, name, strTemp, comment);
}

void CConfOpt::SetValue(const char *section, const char *name, const char *value, const char *comment)
{
    char strSection[256], strName[128], strValue[128];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name))
        return;

    Format(section, strSection);
    Format(name, strName);
    if (value)
        strcpy(strValue, value);
    if (NULL == (m_cSection = SearchSection(strSection)))
    {
        m_cSection = new CSection(strSection, 0);
        m_dcmSection->Push(m_cSection);
    }

    m_bConfigChange = 1;
    if (IS_STR_VALID(comment))
        m_chainComment = new CComment(comment);
    m_cSection->SetValue(strSection, strName, strValue, m_chainComment);
}

void CConfOpt::AddSection(const char *section, int column, const char *comment)
{
    m_cSection = new CSection(section, column);
    m_dcmSection->Push(m_cSection);
    m_bConfigChange = 1;

    m_chainComment = new CComment("######################################################################");
    m_cSection->SetComment(m_chainComment);

    if (IS_STR_VALID(comment))
    {
        m_chainComment = new CComment(comment);
        m_cSection->SetComment(m_chainComment);
    }
}

//得到枚举型配置项目的个数
int CConfOpt::GetCount(const char *section, const char *comment)
{
    return GetCount(section, 0, comment);
}

int CConfOpt::GetCount(const char *section, int column, const char *comment)
{
    char strSection[256];
    int nCount;

    if (!IS_STR_VALID(section))
        return 0;
    Format(section, strSection);
    if (NULL == (m_cSection = SearchSection(strSection)))
    {
        //配置文件中没有这个标题项，生成这个标题项
        if (NULL == comment)
        {
            comment = "请在标题下面输入枚举型变量的值，以空格、回车或TAB分隔";
        }
        AddSection(strSection, column, comment);
        return 0;
    }
    else
    {
        nCount = m_cSection->EnumValueNum(column);
        /*
        if (nCount == 0 && NULL != comment)
        {
            m_cSection->SetComment(comment);
            m_bConfigChange = 1;
        }
        */
        return nCount;
    }
}

void CConfOpt::SetDefaultPrompt(const char *name, const char *defv)
{
    if (m_bItemExist)
        return;

    char strComment[128];
    sprintf(strComment, " %s default is \"%s\"", name, defv);
    m_chainComment = new CComment(strComment);
    m_cSection->SetComment(m_chainComment);
}

//得到枚举型配置项目的第N个值
int CConfOpt::GetValue(const char *section, int index, char *value, const char *comment)
{
    char strSection[256], *pTemp = NULL;

    if (!IS_STR_VALID(section) || !value)
        return 0;
    Format(section, strSection);
    if (NULL == (m_cSection = SearchSection(strSection)))
    {
        //配置文件中没有这个标题项，生成这个标题项
        AddSection(strSection, 0, comment);
        return 0;
    }

    pTemp = m_cSection->GetValue(index);
    if (pTemp)
    {
        strcpy(value, pTemp);
        return 1;
    }
    else
    {
        *value = 0;
        return 0;
    }
}

#define GET_CONF_VALUE(format)                           \
    {                                                    \
        char strValue[128];                              \
        if (!GetValue(section, name, strValue, comment)) \
        {                                                \
            value = defv;                                \
            if (m_bDisplayConfigValue && !m_bItemExist)  \
                SetValue(section, name, defv, NULL);     \
            if (m_bDisplayDefaultPrompt)                 \
            {                                            \
                sprintf(strValue, format, defv);         \
                SetDefaultPrompt(name, strValue);        \
            }                                            \
            return 0;                                    \
        }                                                \
        else                                             \
        {                                                \
            sscanf(strValue, format, &value);            \
            return 1;                                    \
        }                                                \
    }

//取整型配置项的值
int CConfOpt::GetValue(const char *section, const char *name, const int defv, int &value, const char *comment)
{
    GET_CONF_VALUE("%d");
    return 1;
}

//取实型配置项的值
int CConfOpt::GetValue(const char *section, const char *name, const float defv, float &value, const char *comment)
{
    GET_CONF_VALUE("%f");
    return 1;
}

//取字节型配置项的值
int CConfOpt::GetValue(const char *section, const char *name, const unsigned char defv, unsigned char &value, const char *comment)
{
    GET_CONF_VALUE("%c");
    return 1;
}

//取字符串型配置项的值
int CConfOpt::GetValue(const char *section, const char *name, const char *defv, char *value, const char *comment)
{
    if (!value)
        return 0;
    if (!GetValue(section, name, value, comment))
    {
        if (IS_STR_VALID(defv))
            strcpy(value, defv);
        else
            *value = 0;
        if (m_bDisplayConfigValue)
            SetValue(section, name, value, NULL);
        if (m_bDisplayDefaultPrompt)
            SetDefaultPrompt(name, value);
        return 0;
    }
    return 1;
}

//取配置项的字符型值
int CConfOpt::GetValue(const char *section, const char *name, char *value, const char *comment)
{
    char strSection[256], strName[128], *pTemp = NULL;

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name) || !value)
        return 0;
    Format(section, strSection);
    Format(name, strName);
    if (NULL == (m_cSection = SearchSection(strSection)))
    {
        //配置文件中没有这个标题项，生成这个标题项
        AddSection(strSection, 0, NULL);
    }
    else
        pTemp = m_cSection->GetValue(strName);

    m_bItemExist = 1;
    if (!pTemp)
    { //配置标题中没有这个配置项，增加配置项，并使用缺省值
        m_bConfigChange = 1;
        m_bItemExist = 0;
        if (IS_STR_VALID(comment))
            m_chainComment = new CComment(comment);
        else
            m_chainComment = NULL;
        m_cSection->SetValue(strSection, strName, NULL, m_chainComment);
        return 0;
    }
    else if (!strcmp(pTemp, ""))
    { //使用缺省值
        return 0;
    }
    else
    {
        strcpy(value, pTemp);
        return 1;
    }
}
//=========================================
// 设置存盘标记：want_write=1 析构时数据将
// 保存到文件中。
void CConfOpt::SetWriteFlag(int want_write)
{
    m_bConfigChange = want_write;
}

int CConfOpt::IsExist(const char *section, const char *name)
{
    char strSection[256], strName[128];

    if (!IS_STR_VALID(section) || !IS_STR_VALID(name))
        return 0;
    Format(section, strSection);
    Format(name, strName);
    if (NULL == (m_cSection = SearchSection(strSection)))
        return 0;
    return (NULL != m_cSection->SearchRecord(strName));
}

int CConfOpt::DeleteSection(const char *section)
{
    char strSection[256];
    if (!IS_STR_VALID(section))
        return 0;
    Format(section, strSection);
    if (NULL == (m_cSection = SearchSection(strSection)))
        return 0;
    m_dcmSection->Pop(m_cSection);
    delete m_cSection;
    m_cSection = NULL;
    return 1;
}

// 清空配置节内容
int CConfOpt::CleanSection(const char *section)
{
    char strSection[256];
    if (!IS_STR_VALID(section))
        return 0;
    Format(section, strSection);
    if (NULL == (m_cSection = SearchSection(strSection)))
        return 0;
    m_cSection->m_dcmRecord->Empty();
    m_cSection->m_dcmEnumValue->Empty();
    DeletePointer(m_cSection->m_chainComment);
    return 1;
}

// 清除全部枚举配置值
int CConfOpt::CleanEnum(const char *section)
{
    char strSection[256];
    if (!IS_STR_VALID(section))
        return 0;
    Format(section, strSection);
    if (NULL == (m_cSection = SearchSection(strSection)))
        return 0;
    m_cSection->m_dcmEnumValue->Empty();
    return 1;
}

#ifdef __cplusplus
extern "C"
{
#endif

    int CGetValueInt(char *szfilename, const char *section, const char *name, int defv, int *value, const char *comment)
    {
        CConfOpt Conf(szfilename, 1);
        int ret = Conf.GetValue(section, name, defv, *value, comment);
        return ret;
    }
    int CGetValueString(char *szfilename, const char *section, const char *name, const char *defv, char *value, const char *comment)
    {
        CConfOpt Conf(szfilename, 1);
        int ret = Conf.GetValue(section, name, defv, value, comment);
        return ret;
    }
    int CGetValueArr(char *szfilename, const char *section, char outarr[100][200])
    {
        int ncount;
        CConfOpt Conf(szfilename, 1);
        ncount = Conf.GetCount(section, "");
        char szName[256] = "\0";
        // 循环取出CDR模块名称
        for (int i = 0; i < ncount; i++)
        {
            memset(szName, 0, sizeof(szName));
            Conf.GetValue(section, i, szName, NULL);
            strcpy(outarr[i], szName);
        }
        return ncount;
    }

    /******************************************
*得到配置文件中枚举类型段的所有取值
*输入:
		配置文件的名字: char* szfilename
		想要取值的段名: const char *section
		每个枚举字符串的最大长度: int max_len，不要超过256		    
*输出:
		所有的取值:char** poutput
		是一个字符串二维数组，在函数内部分配，
		调用都就在使用完负责释放。
*返回值:
		该段中的枚举个数
*******************************************/

    int CGetEnumValueArr(char *szfilename, const char *section, char **poutput, int max_len)
    {
        int ncount;
        CConfOpt Conf(szfilename, 1);
        char szName[MAX_ENUM_STR_VALUE_LEN] = "\0";
        char *ptemp = NULL;
        ncount = Conf.GetCount(section, "");
        if (ncount <= 0)
        {
            *poutput = NULL;
            return 0;
        }

        *poutput = (char *)malloc(ncount * max_len);
        ptemp = *poutput;

        for (int i = 0; i < ncount; i++)
        {
            memset(szName, 0, MAX_ENUM_STR_VALUE_LEN);
            Conf.GetValue(section, i, szName, NULL);
            strncpy(ptemp, szName, max_len - 1);
            ptemp += max_len;
        }

        return ncount;
    }

    int CSetEnumValue(char *szfilename, const char *section, const char *value)
    {
        CConfOpt Conf(szfilename, 1);
        Conf.SetEnumValue(section, value, NULL);
        Conf.SetWriteFlag(1);
        Conf.WriteToFile();
        return 0;
    }

#ifdef __cplusplus
}
#endif
