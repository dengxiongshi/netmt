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
    CtEM_ALLOW  =0,     //���������ͬ�ڵ�
    CtEM_REJECT =77701, //�����������ͬ�ڵ�
    CtEM_REPLACE        //�滻��ͬ�ڵ�
} CtEqualMethod;

#ifdef CtRetCode
#undef CtRetCode
#endif
typedef enum _chain_template_return_code
{
    CtRC_FAIL       =0,             //����ʧ��
    CtRC_SUCCESS    =1,             //����ɹ�
    CtRC_REJECT     =CtEM_REJECT,   //��������ͬ�ڵ�
    CtRC_REPLACE    =CtEM_REPLACE   //��ͬ�ڵ㱻�滻
} CtRetCode;

#ifdef CtCompareStyle
#undef CtCompartStyle
#endif
typedef enum _chain_template_compare_style
{
    CtCS_GT = 0,    //����
    CtCS_GE = 1,    //���ڵ���
    CtCS_EQ = 2,    //����
    CtCS_NE = 3,    //������
    CtCS_LT = 4,    //С��
    CtCS_LE = 5     //С�ڵ���
} CtCompartStyle;
#endif //_TemplateChainDefine_H
