#include <string.h>
#include "inifileopt.h"

FILE * OpenFile(const char *filename, const char *style, int maxsize)
{
    char strMod[5], *pTemp;
    FILE *fpTemp = NULL;

    if( !filename || !strcmp(filename, "") ) return NULL;
    if( !style || !strcmp(style, "") ) strcpy(strMod, "r");
    else strcpy( strMod, style);

    if( !strcmp(strMod, "r") || !strcmp(strMod, "rb") )
    { //open file for reading
        if( !IsFile(filename) ) return NULL;
        return fopen(filename, strMod);
    }
    else if( !strcmp(strMod, "w") || !strcmp(strMod, "wb")
        || !strcmp(strMod, "w+") || !strcmp(strMod, "wb+") || !strcmp(strMod, "w+b") )
    { //truncate to zero length or create file for writing
        return fopen(filename, strMod);
    }
    else if( !strcmp(strMod, "a") || !strcmp(strMod, "ab")
        || !strcmp(strMod, "a+") || !strcmp(strMod, "ab+") || !strcmp(strMod, "a+b")
        || !strcmp(strMod, "r+") || !strcmp(strMod, "rb+") || !strcmp(strMod, "r+b") )
    { //open or create file for writing at end of file
        fpTemp = fopen(filename, strMod);
        if( !fpTemp && (pTemp=strchr(strMod, 'r')) )
        {
            *pTemp = 'w';
            fpTemp = fopen(filename, strMod);
        }

        if( fpTemp && maxsize && !strchr(strMod, 'w') )
        {
            fseek(fpTemp, 0, SEEK_END);
            if( ftell(fpTemp) > maxsize*1024 )
            {
                fclose(fpTemp);
                fpTemp=fopen(filename, "w");
            }
        }
        return fpTemp;
    }
    else return NULL;
}

void CloseFile(FILE * &fp)
{
    if( fp && fp != stdout && fp != stdin && fp != stderr ) fclose(fp);
    fp = NULL;
}



#ifdef WIN32
#define W_OK 2
#endif

int FileType(const char *file)
{
    struct stat s_buf;

    if( !file || !strcmp(file, "") ) return -1;
    /*
    ** -1: stat error
    **  0: other
    **  1: regular file
    **  2: directory
    */
    if( stat(file, &s_buf) == -1) return -1;
    else if( (s_buf.st_mode&S_IFMT) == S_IFDIR) return 2;
    else if( !(s_buf.st_mode&S_IFREG) || access(file, W_OK) == -1) return 0;
    return 1;
}

int IsFile(const char *filename)
{
    return (FileType(filename) == 1 )?1:0;
}

int IsDir(const char *directory)
{
    return (FileType(directory) == 2 )?1:0;
}

int WriteStrToFile(const char *filename, const char * text, int maxsize, int writetime)
{
    char m_strTemp[128];
    FILE * m_fpFile;
    int nFileSize;

    if( !filename || !strcmp(filename, "") || !text || !strcmp(text, "") ) return -1;

    if( NULL == (m_fpFile = fopen(filename,"a")) )
    {
        m_fpFile = fopen(filename,"w");
        if( m_fpFile == NULL ) return -1;
    }
    fseek(m_fpFile,0,SEEK_END);

    nFileSize = ftell(m_fpFile);
    
    if( nFileSize > maxsize*1024 )
    {
        //备份数据到 .old 文件
        fclose(m_fpFile);
        sprintf(m_strTemp, "mv %s %s.Old", filename, filename);
        system(m_strTemp);
        m_fpFile = fopen(filename,"w");
        if( m_fpFile == NULL ) return -1;
    }

    if( writetime )
    {
        time_t curTime;
        curTime = time(&curTime);
        struct tm * curTm = localtime(&curTime);
    
        fprintf(m_fpFile, "\nLOG TIME:%04d/%02d/%02d %02d:%02d:%02d ::\n",
            curTm->tm_year + 1900, curTm->tm_mon + 1, curTm->tm_mday,
            curTm->tm_hour, curTm->tm_min, curTm->tm_sec);
    }

    fprintf( m_fpFile, "%s\n", text);
    fclose(m_fpFile);

    return 1;
}

bool IsFileLocked(const char * strFile)
{
    int fd, retCode;

    struct flock  lock_stru;

    if ((fd=open(strFile, O_RDWR | O_CREAT, 0666))<0)
    {
        //打开失败,返回成功
        return true;
    }
    else//打开成功
    {
        lock_stru.l_type=F_WRLCK;  //置写锁
        lock_stru.l_whence=0;      //开始类型(从文件头开始)
        lock_stru.l_start=1;       //开始偏移
        lock_stru.l_len=0;         //块大小,0代表锁整个文件
        if((retCode=fcntl(fd, F_SETLK, &lock_stru))!=-1) //锁成功
        {
        	close(fd);
            return false;
        }
    }
	close(fd);
    return true;
}

