#ifndef _DoubleDirectChainTemplateClass_H
#define _DoubleDirectChainTemplateClass_H

#include "initchaindefine.h"

////////////////////////////////////////////////////////////////////////////////
//˫��ѭ��������//////////////////////////////////////////////////////////////
//sort ָʾ�Ƿ�����
//equal ָʾ��������ͬ�ڵ�ʱ�Ĵ�����
//pointer ָʾ�Ƿ���һ��ָ�����ͣ�������ʱ�ز�����
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDChain
{
public:
    CTDChain<T, sort, equal, pointer> *m_pPrev;
    CTDChain<T, sort, equal, pointer> *m_pNext;
    T m_tItem;

protected:
    //================================================================
    // ������ͷ����һ���ڵ�, ʧ��ʱ���ؿ�ָ�룬directָʾ��������
    virtual CTDChain *SearchPrev(T &item)
    {
        if (TypeComp(m_tItem, item, CtCS_EQ))
            return this;
        else if (m_pPrev)
            return m_pPrev->CTDChain<T, sort, equal, pointer>::SearchPrev(item);
        else
            return NULL;
    }

    //================================================================
    // ������β������һ���ڵ�, ʧ��ʱ���ؿ�ָ�룬directָʾ��������
    virtual CTDChain *SearchNext(T &item)
    {
        if (TypeComp(m_tItem, item, CtCS_EQ))
            return this;
        else if (m_pNext)
            return m_pNext->CTDChain<T, sort, equal, pointer>::SearchNext(item);
        else
            return NULL;
    }

    //================================================================
    //�ӱ��ڵ㿪ʼ��������С�Ľڵ㣬�����򺯵���
    virtual CTDChain *SmallestNext()
    {
        CTDChain<T, sort, equal, pointer> *nodeNext = m_pNext;
        CTDChain<T, sort, equal, pointer> *nodeSmall = this;

        while (nodeNext)
        {
            if (nodeSmall->m_tItem > nodeNext->m_tItem)
                nodeSmall = nodeNext;
            nodeNext = nodeNext->m_pNext;
        }
        return nodeSmall;
    }

public:
    //================================================================
    CTDChain() : m_pPrev(0), m_pNext(0), m_tItem(0){};

    //================================================================
    CTDChain(T &item) : m_pPrev(0), m_pNext(0), m_tItem(0)
    {
        CTDChain<T, sort, equal, pointer>::SetNodeItem(item);
    }

    //================================================================
    //�������캯��
    CTDChain(CTDChain &node)
    {
        m_pPrev = node.m_pPrev;
        m_pNext = node.m_nNext;
        if (pointer)
        {
            if (!m_tItem)
                m_tItem = new T;
            *m_tItem = *(node.nodeItem);
        }
        else
            m_tItem = node.nodeItem;
    }

    //================================================================
    //����ʱ�Զ�ɾ����������Ľڵ�
    ~CTDChain()
    {
        if (pointer && m_tItem)
            delete m_tItem;
        if (m_pPrev)
            m_pPrev->m_pNext = NULL;
        if (m_pNext)
        {
            m_pNext->m_pPrev = NULL;
            delete m_pNext;
            m_pNext = NULL;
        }
    }

    //================================================================
    // �ҵ�����ͷ�ڵ�
    CTDChain *Head()
    {
        if (!m_pPrev)
            return this;
        else
            return m_pPrev->CTDChain<T, sort, equal, pointer>::Head();
    }

    //================================================================
    // �ҵ�����β�ڵ�
    CTDChain *Tail()
    {
        if (!m_pNext)
            return this;
        else
            return m_pNext->CTDChain<T, sort, equal, pointer>::Tail();
    }

    //================================================================
    //ͳ�������нڵ����
    int NodeNum()
    {
        int nNum = 0;
        CTDChain<T, sort, equal, pointer> *nodeTemp = Head();
        while (nodeTemp)
        {
            nNum++;
            nodeTemp = nodeTemp->m_pNext;
        }
        return nNum;
    }

    //================================================================
    //������������һ���ڵ�
    //ע�⣺Popͷ�ڵ�ʱ��Ҫ��ָ���¼�µ�ͷ�ڵ�
    virtual void Pop()
    {
        if (m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        if (m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        m_pPrev = m_pNext = NULL;
    }

    //================================================================
    //����һ���ڵ��ֵ���滻��ԭ�е�ֵ
    virtual int SetNodeItem(const T &item)
    {
        if (pointer && m_tItem)
            delete m_tItem;
        m_tItem = item;
        return CtRC_REPLACE;
    }

    int TypeComp(const T &a, const T &b, CtCompartStyle style = CtCS_GT)
    {
        //int nRet = 0;

        if (pointer)
        {
            switch (style)
            {
            case CtCS_GT:
                return ((*a) > (*b));
                break;
            case CtCS_GE:
                return ((*a) >= (*b));
                break;
            case CtCS_EQ:
                return ((*a) == (*b));
                break;
            case CtCS_NE:
                return ((*a) != (*b));
                break;
            case CtCS_LT:
                return ((*a) < (*b));
                break;
            case CtCS_LE:
                return ((*a) <= (*b));
                break;
            default:
                return 0;
            }
        }
        else
        {
            switch (style)
            {
            case CtCS_GT:
                return (a > b);
                break;
            case CtCS_GE:
                return (a >= b);
                break;
            case CtCS_EQ:
                return (a == b);
                break;
            case CtCS_NE:
                return (a != b);
                break;
            case CtCS_LT:
                return (a < b);
                break;
            case CtCS_LE:
                return (a <= b);
                break;
            default:
                return 0;
            }
        }
    }

    //================================================================
    // ����һ���ڵ�, ʧ��ʱ���ؿ�ָ��
    virtual CTDChain *Search(T &item)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp = NULL;

        if (TypeComp(m_tItem, item, CtCS_EQ))
            return this;
        if (sort)
        { //����
            if (TypeComp(item, m_tItem))
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchNext(item);
            else
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
        }
        else
        { //����
            nodeTemp = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
            if (!nodeTemp)
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchNext(item);
        }
        return nodeTemp;
    }

    //================================================================
    //����һ���ڵ��ڱ��ڵ�֮ǰ��node������һ�����ڵ�
    virtual int PushPrev(CTDChain *node)
    {
        node->m_pNext = this;
        node->m_pPrev = m_pPrev;
        if (m_pPrev)
            m_pPrev->m_pNext = node;
        m_pPrev = node;
        return CtRC_SUCCESS;
    }

    //================================================================
    //����һ���ڵ��ڱ��ڵ�֮��node������һ�����ڵ�
    virtual int PushNext(CTDChain *node)
    {
        node->m_pNext = m_pNext;
        node->m_pPrev = this;
        if (m_pNext)
            m_pNext->m_pPrev = node;
        m_pNext = node;
        return CtRC_SUCCESS;
    }

    //================================================================
    //����һ�����ڵ�
    virtual int Push(CTDChain *&node)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;

        if (!node)
            return CtRC_FAIL;

        //�ж���ͬ�ڵ�
        switch (equal)
        {
        case CtEM_REJECT:  //�����������ͬ�ڵ�
        case CtEM_REPLACE: //�滻��ͬ�ڵ�
            nodeTemp = CTDChain<T, sort, equal, pointer>::Search(node->m_tItem);
            if (NULL == nodeTemp)
                break;
            if (equal == CtEM_REJECT)
                return CtRC_REJECT;
            return nodeTemp->CTDChain<T, sort, equal, pointer>::SetNodeItem(node->m_tItem);
            break;
        case CtEM_ALLOW: //���������ͬ�ڵ�
        default:
            break;
        }

        //���Ҳ���㣬���ﲻ�ÿ���������
        nodeTemp = this;
        if (sort)
        { //����
            if (TypeComp(node->m_tItem, nodeTemp->m_tItem))
            { //���β����
                while (TypeComp(node->m_tItem, nodeTemp->m_tItem))
                {
                    if (!nodeTemp->m_pNext)
                        break;
                    nodeTemp = nodeTemp->m_pNext;
                }
            }
            else
            { //���ͷ����
                while (TypeComp(nodeTemp->m_tItem, node->m_tItem))
                {
                    if (!nodeTemp->m_pPrev)
                        return nodeTemp->CTDChain<T, sort, equal, pointer>::PushPrev(node);
                    nodeTemp = nodeTemp->m_pPrev;
                }
            }
        }
        else
        { //����
            nodeTemp = CTDChain<T, sort, equal, pointer>::Tail();
        }

        //�����滹��һ������㣬�ǲ����������ͷ��
        return nodeTemp->CTDChain<T, sort, equal, pointer>::PushNext(node);
    }

    virtual int Push(T &item)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;
        int nRet;

        if (NULL == (nodeTemp = new CTDChain<T, sort, equal, pointer>(item)))
        {
            RAISE_ERROR("���棺�����ڴ�ʧ��");
            return CtRC_FAIL;
        }

        nRet = CTDChain<T, sort, equal, pointer>::Push(nodeTemp);
        switch (nRet)
        {
        case CtRC_FAIL:
            break;
        case CtRC_SUCCESS:
            break;
        case CtRC_REPLACE:
            if (pointer)
                nodeTemp->m_tItem = NULL;
        case CtRC_REJECT:
            delete nodeTemp;
            break;
        default:
            break;
        }

        return nRet;
    }

    //================================================================
    //��������һ����������нڵ㣬����ֵ�ǲ���ڵ�ĸ���
    //������ɺ�ԭ����ɾ��
    virtual int Merge(CTDChain *&chain)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;
        int nRet, nNum = 0;
        if (!chain)
            return 0;

        //�Ƚض�ԭ����
        if (chain->m_pPrev)
        {
            chain->m_pPrev->m_pNext = NULL;
            chain->m_pPrev = NULL;
        }

        //���������ͬ�ڵ�������
        if (equal == CtEM_ALLOW && !sort)
        {
            nNum = chain->NodeNum();
            nodeTemp = CTDChain<T, sort, equal, pointer>::Tail();
            nodeTemp->m_pNext = chain;
            chain->m_pPrev = nodeTemp;
        }
        else
        {
            nodeTemp = chain;
            while (nodeTemp)
            {
                chain = chain->m_pNext;
                nodeTemp->Pop();
                nRet = CTDChain<T, sort, equal, pointer>::Push(nodeTemp);
                switch (nRet)
                {
                case CtRC_SUCCESS:
                    nNum++;
                    break;
                case CtRC_REPLACE:
                    if (pointer)
                        nodeTemp->m_tItem = NULL;
                case CtRC_REJECT:
                case CtRC_FAIL:
                    delete nodeTemp;
                    break;
                default:
                    break;
                }
                nodeTemp = chain;
            }
        }
        chain = NULL;
        return nNum;
    }

    //================================================================
    //������, ��ʹ��T�Ĳ����� '>'
    virtual void Sort()
    {
        CTDChain<T, sort, equal, pointer> *nodeSmall;
        CTDChain<T, sort, equal, pointer> *nodeTemp = CTDChain<T, sort, equal, pointer>::Head();

        while (nodeTemp)
        {
            nodeSmall = nodeTemp->CTDChain<T, sort, equal, pointer>::SmallestNext();
            if (nodeSmall != nodeTemp)
            { //��С�ڵ㲻�ǵ�ǰ�ڵ�
                nodeSmall->CTDChain<T, sort, equal, pointer>::Pop();
                nodeTemp->CTDChain<T, sort, equal, pointer>::PushPrev(nodeSmall);
            }
            else
                nodeTemp = nodeTemp->m_pNext;
        }
    }

    //================================================================
    //���صĲ�����
    virtual int operator>(const CTDChain &right)
    {
        return TypeComp(m_tItem, right.m_tItem, CtCS_GT);
    }

    virtual int operator<(const CTDChain &right)
    {
        return TypeComp(m_tItem, right.m_tItem, CtCS_LT);
    }

    virtual int operator==(const CTDChain &right)
    {
        return TypeComp(m_tItem, right.m_tItem, CtCS_EQ);
    }

    virtual void operator=(const CTDChain &right)
    {
        CTDChain<T, sort, equal, pointer>::SetNodeItem(right.m_tItem);
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//˫��ѭ��������////////////////////////////////////////////////////////////////
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDLChain : public CTDChain<T, sort, equal, pointer>
{
protected:
    //================================================================
    // �򿪻�
    void OpenCircle()
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev->m_pNext = NULL;
        CTDChain<T, sort, equal, pointer>::m_pPrev = NULL;
    }

    //================================================================
    // �رջ�
    void CloseCircle()
    {
        CTDLChain<T, sort, equal, pointer> *tail = CTDChain<T, sort, equal, pointer>::Tail();
        CTDLChain<T, sort, equal, pointer> *head = CTDChain<T, sort, equal, pointer>::Head();
        tail->m_pNext = head;
        head->m_pPrev = tail;
    }

    //================================================================
    // ��ǰ����һ���ڵ�, ʧ��ʱ���ؿ�ָ��
    CTDLChain *SearchPrev(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;
        OpenCircle();
        node = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
        CloseCircle();
        return node;
    }

    //================================================================
    // �������һ���ڵ�, ʧ��ʱ���ؿ�ָ��
    CTDLChain *SearchNext(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;
        OpenCircle();
        node = CTDChain<T, sort, equal, pointer>::SearchNext(item);
        CloseCircle();
        return node;
    }

public:
    //================================================================
    CTDLChain() : CTDChain<T, sort, equal, pointer>()
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev = CTDChain<T, sort, equal, pointer>::m_pNext = this;
    }

    //================================================================
    CTDLChain(T &item) : CTDChain<T, sort, equal, pointer>(item)
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev = CTDChain<T, sort, equal, pointer>::m_pNext = this;
        CTDChain<T, sort, equal, pointer>::SetNodeItem(item);
    }

    //================================================================
    //�������캯��
    CTDLChain(CTDLChain &node) : CTDChain<T, sort, equal, pointer>(node)
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev = node.m_pPrev;
        CTDChain<T, sort, equal, pointer>::m_pNext = node.m_pNext;
    }

    //================================================================
    //����ʱ�Զ���������ժ��
    ~CTDLChain()
    {
    }

    //================================================================
    // ����һ���ڵ�, ʧ��ʱ���ؿ�ָ��
    CTDLChain *Search(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;

        OpenCircle();
        node = CTDLChain<T, sort, equal, pointer>::Search(item);
        CloseCircle();
        return node;
    }

    //================================================================
    //����һ���ڵ㣬����ֵ
    int Push(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;

        if (sort)
            node = this;
        else
            node = CTDChain<T, sort, equal, pointer>::m_pPrev;

        int nRet;
        OpenCircle();
        nRet = node->CTDChain<T, sort, equal, pointer>::Push(item);
        CloseCircle();
        return nRet;
    }

    //================================================================
    //��������һ������
    void Merge(CTDLChain *&chain)
    {
        if (!chain)
            return;
        OpenCircle();
        chain->OpenCircle();
        CTDChain<T, sort, equal, pointer>::Merge(chain);
        CloseCircle();
    }

    //================================================================
    //������, ��ʹ��T�Ĳ����� '>'
    void Sort()
    {
        OpenCircle();
        CTDChain<T, sort, equal, pointer>::Sort();
        CloseCircle();
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////˫��ѭ������ģ�������//////////////////////////////////////////////////
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDChainMgr
{
    // ���ݳ�Ա
public:
    CTDChain<T, sort, equal, pointer> *m_nodeHead; // ����ͷ�ڵ�ָ��
    CTDChain<T, sort, equal, pointer> *m_nodeTail; // ����β�ڵ�ָ��
    CTDChain<T, sort, equal, pointer> *m_nodeCurr; // ����ǰ�ڵ�ָ��

    int m_nNodeNum; //����ڵ����

    // ������Ա
public:
    //================================================================
    CTDChainMgr(void) : m_nodeHead(0), m_nodeTail(0), m_nodeCurr(0), m_nNodeNum(0){};

    //================================================================
    ~CTDChainMgr()
    {
        if (this == NULL)
        {
            RAISE_ERROR("���棺����ʹ��һ��δ��ʼ������ָ��");
            exit(0);
        }
        Empty();
    }

    //================================================================
    // ȥ��һ���ڵ㣬�ͷ�����ڵ�Ŀռ䣬�����ͷŽڵ����ݿռ�
    void Pop(T &item)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp = m_nodeHead->Search(item);

        if (!nodeTemp)
            return;
        else if (nodeTemp == m_nodeHead)
            m_nodeHead = m_nodeHead->m_pNext;
        else if (nodeTemp == m_nodeTail)
            m_nodeTail = m_nodeTail->m_pPrev;

        nodeTemp->Pop();
        delete nodeTemp;
        m_nNodeNum--;
    }

    //================================================================
    //���������ɾ���ڵ���
    void Null()
    {
        m_nodeHead = m_nodeTail = m_nodeCurr = NULL;
        m_nNodeNum = 0;
    }

    //================================================================
    //�������ͬʱɾ���ڵ���
    void Empty()
    {
        if (m_nodeHead)
            delete m_nodeHead;
        Null();
    }

    //================================================================
    //�õ�ǰ�ڵ�ָ��(m_nodeCurr)Ϊ����ͷ
    T Begin()
    {
        if (!m_nodeHead)
            return NULL;
        m_nodeCurr = m_nodeHead;
        return m_nodeHead->m_tItem;
    }

    //================================================================
    //�������һ���ڵ㣬���õ�ǰ�ڵ�ָ��Ϊ����β
    T End()
    {
        if (!m_nodeHead)
            return NULL;
        m_nodeCurr = m_nodeTail;
        return m_nodeTail->m_tItem;
    }

    //================================================================
    //����ƶ���ǰ�ڵ�ָ�룬�����ص�ǰλ�ýڵ�����
    T Next()
    {
        m_nodeCurr = m_nodeCurr->m_pNext;
        if (!m_nodeCurr)
        {
            m_nodeCurr = m_nodeHead;
            return NULL;
        }
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    //��ǰ�ƶ���ǰ�ڵ�ָ�룬�����ص�ǰλ�ýڵ�����
    T Prev()
    {
        m_nodeCurr = m_nodeCurr->m_pPrev;
        if (!m_nodeCurr)
        {
            m_nodeCurr = m_nodeTail;
            return NULL;
        }
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    //��������һ��˫��ѭ�������ָ���ڵ��Ժ�����нڵ㣬����ֵ�Ǳ���
    //��ڵ���ܸ���
    int Push(CTDChain<T, sort, equal, pointer> *&chain)
    {
        m_nNodeNum += m_nodeTail->CTDChain<T, sort, equal, pointer>::Merge(chain);
        m_nodeHead = m_nodeHead->Head();
        m_nodeTail = m_nodeTail->Tail();
        return m_nNodeNum;
    }

    //================================================================
    // ��������һ���������������нڵ㣬����ֵ�Ǳ�����ڵ���ܸ���
    int Push(CTDChainMgr *chainmgr)
    {
        if (!chainmgr || !chainmgr->m_nNodeNum)
            return 0;
        Push(chainmgr->m_nodeHead);
        chainmgr->Empty(); //
        return m_nNodeNum;
    }

    //================================================================
    // ����һ���ڵ㣬����ֵָʾ�����Ƿ�ɹ�
    int Push(T &item)
    {
        if (!m_nodeHead)
        {
            m_nodeHead = new CTDChain<T, sort, equal, pointer>(item);
            m_nodeTail = m_nodeCurr = m_nodeHead;
            m_nNodeNum++;
            return CtRC_SUCCESS;
        }

        if (CtRC_SUCCESS == m_nodeTail->Push(item))
        {
            m_nodeHead = m_nodeHead->Head();
            m_nodeTail = m_nodeTail->Tail();
            m_nNodeNum++;
            return CtRC_SUCCESS;
        }
        return CtRC_FAIL;
    }

    //================================================================
    // ����һ���ڵ�
    int IsInChain(T &item)
    {
        return (m_nodeCurr->Search(item) == NULL) ? 0 : 1;
    }

    //================================================================
    //����
    void Sort()
    {
        m_nodeHead->Sort();
        m_nodeHead = m_nodeHead->Head();
        m_nodeTail = m_nodeTail->Tail();
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////˫��ѭ������ģ�������//////////////////////////////////////////////////
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDLChainMgr
{
private:
    CTDLChain<T, sort, equal, pointer> *m_nodeHead;
    CTDLChain<T, sort, equal, pointer> *m_nodeCurr;

    int m_nNodeNum;

public:
    //================================================================
    CTDLChainMgr() : m_nodeHead(0), m_nodeCurr(0), m_nNodeNum(0){};

    //================================================================
    ~CTDLChainMgr()
    {
        if (m_nodeHead)
        {
            m_nodeHead->OpenCircle();
            delete m_nodeHead;
        }
        m_nodeHead = m_nodeCurr = NULL;
        m_nNodeNum = 0;
    }

    //================================================================
    //�õ�ǰ�ڵ�ָ��(m_nodeCurr)Ϊ����ͷ
    T Begin()
    {
        m_nodeCurr = m_nodeHead;
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    //����ƶ���ǰ�ڵ�ָ�룬������ǰһλ�ýڵ�����
    T Next()
    {
        m_nodeCurr = m_nodeCurr->m_pNext;
        if (m_nodeCurr == m_nodeHead)
            return NULL;
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    // ����һ���ڵ�
    int Push(T &item)
    {
        if (!m_nodeHead)
        {
            m_nodeHead = new CTDLChain<T, sort, equal, pointer>(item);
            m_nodeCurr = m_nodeHead;
            m_nNodeNum++;
            return CtRC_SUCCESS;
        }

        if (CtRC_SUCCESS == m_nodeHead->Push(item))
        {
            m_nNodeNum++;
            return CtRC_SUCCESS;
        }

        return CtRC_FAIL;
    }

    //================================================================
    // ȥ��һ���ڵ㣬�ͷ�����ڵ�Ŀռ�
    void Pop(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node = m_nodeHead->Search(item);

        if (node == m_nodeHead)
            m_nodeHead = m_nodeHead->m_pNext;
        delete node;
        m_nNodeNum--;
    }

    //================================================================
    // ����һ���ڵ�
    int IsInChain(T &item)
    {
        return (NULL == m_nodeCurr->Search(item)) ? 0 : 1;
    }

    //================================================================
    //����
    void Sort()
    {
        m_nodeHead->Sort();
    }
};

#endif //_DoubleDirectChainTemplateClass_H
