#ifndef _TemplateChainDefine_H
#define _TemplateChainDefine_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RAISE_ERROR
#undef RAISE_ERROR
#endif
#define RAISE_ERROR(string) printf("[%s:%d] %s\n", __FILE__, __LINE__, (string))

#ifdef CtEqualMethod
#undef CtEqualMethod
#endif
typedef enum _chain_template_equal_method
{
    CtEM_ALLOW  =0,     //允许出现相同节点
    CtEM_REJECT =77701, //不允许出现相同节点
    CtEM_REPLACE        //替换相同节点
} CtEqualMethod;

#ifdef CtRetCode
#undef CtRetCode
#endif
typedef enum _chain_template_return_code
{
    CtRC_FAIL       =0,             //插入失败
    CtRC_SUCCESS    =1,             //插入成功
    CtRC_REJECT     =CtEM_REJECT,   //不允许相同节点
    CtRC_REPLACE    =CtEM_REPLACE   //相同节点被替换
} CtRetCode;

#ifdef CtCompareStyle
#undef CtCompartStyle
#endif
typedef enum _chain_template_compare_style
{
    CtCS_GT = 0,    //大于
    CtCS_GE = 1,    //大于等于
    CtCS_EQ = 2,    //等于
    CtCS_NE = 3,    //不等于
    CtCS_LT = 4,    //小于
    CtCS_LE = 5     //小于等于
} CtCompartStyle;
#endif //_TemplateChainDefine_H
