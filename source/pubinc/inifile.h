#ifndef __INIFILE_H
#define __INIFILE_H

// #include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int CGetValueInt(char* szfilename,const char *section, const char *name, int defv, int *value, const char *comment);
extern int CGetValueString(char* szfilename,const char *section, const char *name, const char * defv, char * value, const char *comment);
extern int CGetValueArr(char* szfilename,const char *section, char outarr[100][200]);
extern int CGetEnumValueArr(char* szfilename,const char *section, char** poutput, int max_len);
extern int CSetEnumValue(char* szfilename,const char *section, const char *value );


#ifdef __cplusplus
}
#endif

#endif