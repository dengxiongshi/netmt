#ifndef _DoubleDirectChainTemplateClass_H
#define _DoubleDirectChainTemplateClass_H

#include "initchaindefine.h"

////////////////////////////////////////////////////////////////////////////////
//双向不循环链表类//////////////////////////////////////////////////////////////
//sort 指示是否有序
//equal 指示当出现相同节点时的处理方法
//pointer 指示是否是一个指针类型，在析构时必不可少
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
    // 向链表头搜索一个节点, 失败时返回空指针，direct指示搜索方向
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
    // 向链表尾部搜索一个节点, 失败时返回空指针，direct指示搜索方向
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
    //从本节点开始向后查找最小的节点，由排序函调用
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
    //考贝构造函数
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
    //析构时自动删除在它后面的节点
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
    // 找到链表头节点
    CTDChain *Head()
    {
        if (!m_pPrev)
            return this;
        else
            return m_pPrev->CTDChain<T, sort, equal, pointer>::Head();
    }

    //================================================================
    // 找到链表尾节点
    CTDChain *Tail()
    {
        if (!m_pNext)
            return this;
        else
            return m_pNext->CTDChain<T, sort, equal, pointer>::Tail();
    }

    //================================================================
    //统计链表中节点个数
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
    //从链表中移走一个节点
    //注意：Pop头节点时，要有指针记录新的头节点
    virtual void Pop()
    {
        if (m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        if (m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        m_pPrev = m_pNext = NULL;
    }

    //================================================================
    //设置一个节点的值，替换掉原有的值
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
    // 搜索一个节点, 失败时返回空指针
    virtual CTDChain *Search(T &item)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp = NULL;

        if (TypeComp(m_tItem, item, CtCS_EQ))
            return this;
        if (sort)
        { //有序
            if (TypeComp(item, m_tItem))
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchNext(item);
            else
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
        }
        else
        { //无序
            nodeTemp = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
            if (!nodeTemp)
                nodeTemp = CTDChain<T, sort, equal, pointer>::SearchNext(item);
        }
        return nodeTemp;
    }

    //================================================================
    //插入一个节点在本节点之前，node必须是一个单节点
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
    //插入一个节点在本节点之后，node必须是一个单节点
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
    //插入一个单节点
    virtual int Push(CTDChain *&node)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;

        if (!node)
            return CtRC_FAIL;

        //判断相同节点
        switch (equal)
        {
        case CtEM_REJECT:  //不允许出现相同节点
        case CtEM_REPLACE: //替换相同节点
            nodeTemp = CTDChain<T, sort, equal, pointer>::Search(node->m_tItem);
            if (NULL == nodeTemp)
                break;
            if (equal == CtEM_REJECT)
                return CtRC_REJECT;
            return nodeTemp->CTDChain<T, sort, equal, pointer>::SetNodeItem(node->m_tItem);
            break;
        case CtEM_ALLOW: //允许出现相同节点
        default:
            break;
        }

        //查找插入点，这里不用考虑相等情况
        nodeTemp = this;
        if (sort)
        { //有序
            if (TypeComp(node->m_tItem, nodeTemp->m_tItem))
            { //向表尾查找
                while (TypeComp(node->m_tItem, nodeTemp->m_tItem))
                {
                    if (!nodeTemp->m_pNext)
                        break;
                    nodeTemp = nodeTemp->m_pNext;
                }
            }
            else
            { //向表头查找
                while (TypeComp(nodeTemp->m_tItem, node->m_tItem))
                {
                    if (!nodeTemp->m_pPrev)
                        return nodeTemp->CTDChain<T, sort, equal, pointer>::PushPrev(node);
                    nodeTemp = nodeTemp->m_pPrev;
                }
            }
        }
        else
        { //无序
            nodeTemp = CTDChain<T, sort, equal, pointer>::Tail();
        }

        //在上面还有一个插入点，是插入在链表的头部
        return nodeTemp->CTDChain<T, sort, equal, pointer>::PushNext(node);
    }

    virtual int Push(T &item)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;
        int nRet;

        if (NULL == (nodeTemp = new CTDChain<T, sort, equal, pointer>(item)))
        {
            RAISE_ERROR("警告：申请内存失败");
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
    //插入另外一个链表的所有节点，返回值是插入节点的个数
    //插入完成后，原链表被删除
    virtual int Merge(CTDChain *&chain)
    {
        CTDChain<T, sort, equal, pointer> *nodeTemp;
        int nRet, nNum = 0;
        if (!chain)
            return 0;

        //先截断原链表
        if (chain->m_pPrev)
        {
            chain->m_pPrev->m_pNext = NULL;
            chain->m_pPrev = NULL;
        }

        //如果允许相同节点且无序
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
    //排序函数, 它使用T的操作符 '>'
    virtual void Sort()
    {
        CTDChain<T, sort, equal, pointer> *nodeSmall;
        CTDChain<T, sort, equal, pointer> *nodeTemp = CTDChain<T, sort, equal, pointer>::Head();

        while (nodeTemp)
        {
            nodeSmall = nodeTemp->CTDChain<T, sort, equal, pointer>::SmallestNext();
            if (nodeSmall != nodeTemp)
            { //最小节点不是当前节点
                nodeSmall->CTDChain<T, sort, equal, pointer>::Pop();
                nodeTemp->CTDChain<T, sort, equal, pointer>::PushPrev(nodeSmall);
            }
            else
                nodeTemp = nodeTemp->m_pNext;
        }
    }

    //================================================================
    //重载的操作符
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
//双向循环链表类////////////////////////////////////////////////////////////////
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDLChain : public CTDChain<T, sort, equal, pointer>
{
protected:
    //================================================================
    // 打开环
    void OpenCircle()
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev->m_pNext = NULL;
        CTDChain<T, sort, equal, pointer>::m_pPrev = NULL;
    }

    //================================================================
    // 关闭环
    void CloseCircle()
    {
        CTDLChain<T, sort, equal, pointer> *tail = CTDChain<T, sort, equal, pointer>::Tail();
        CTDLChain<T, sort, equal, pointer> *head = CTDChain<T, sort, equal, pointer>::Head();
        tail->m_pNext = head;
        head->m_pPrev = tail;
    }

    //================================================================
    // 向前搜索一个节点, 失败时返回空指针
    CTDLChain *SearchPrev(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;
        OpenCircle();
        node = CTDChain<T, sort, equal, pointer>::SearchPrev(item);
        CloseCircle();
        return node;
    }

    //================================================================
    // 向后搜索一个节点, 失败时返回空指针
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
    //考贝构造函数
    CTDLChain(CTDLChain &node) : CTDChain<T, sort, equal, pointer>(node)
    {
        CTDChain<T, sort, equal, pointer>::m_pPrev = node.m_pPrev;
        CTDChain<T, sort, equal, pointer>::m_pNext = node.m_pNext;
    }

    //================================================================
    //析构时自动从链表中摘除
    ~CTDLChain()
    {
    }

    //================================================================
    // 搜索一个节点, 失败时返回空指针
    CTDLChain *Search(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node;

        OpenCircle();
        node = CTDLChain<T, sort, equal, pointer>::Search(item);
        CloseCircle();
        return node;
    }

    //================================================================
    //插入一个节点，返回值
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
    //插入另外一个链表，
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
    //排序函数, 它使用T的操作符 '>'
    void Sort()
    {
        OpenCircle();
        CTDChain<T, sort, equal, pointer>::Sort();
        CloseCircle();
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////双向不循环链表模板管理类//////////////////////////////////////////////////
template <class T,
          const int sort = 0,
          const CtEqualMethod equal = CtEM_ALLOW,
          const int pointer = 1>
class CTDChainMgr
{
    // 数据成员
public:
    CTDChain<T, sort, equal, pointer> *m_nodeHead; // 链表头节点指针
    CTDChain<T, sort, equal, pointer> *m_nodeTail; // 链表尾节点指针
    CTDChain<T, sort, equal, pointer> *m_nodeCurr; // 链表当前节点指针

    int m_nNodeNum; //链表节点个数

    // 函数成员
public:
    //================================================================
    CTDChainMgr(void) : m_nodeHead(0), m_nodeTail(0), m_nodeCurr(0), m_nNodeNum(0){};

    //================================================================
    ~CTDChainMgr()
    {
        if (this == NULL)
        {
            RAISE_ERROR("警告：不能使用一个未初始化的类指针");
            exit(0);
        }
        Empty();
    }

    //================================================================
    // 去掉一个节点，释放这个节点的空间，但不释放节点内容空间
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
    //清空链表，不删除节点类
    void Null()
    {
        m_nodeHead = m_nodeTail = m_nodeCurr = NULL;
        m_nNodeNum = 0;
    }

    //================================================================
    //清空链表，同时删除节点类
    void Empty()
    {
        if (m_nodeHead)
            delete m_nodeHead;
        Null();
    }

    //================================================================
    //置当前节点指针(m_nodeCurr)为链表头
    T Begin()
    {
        if (!m_nodeHead)
            return NULL;
        m_nodeCurr = m_nodeHead;
        return m_nodeHead->m_tItem;
    }

    //================================================================
    //返回最后一个节点，并置当前节点指针为链表尾
    T End()
    {
        if (!m_nodeHead)
            return NULL;
        m_nodeCurr = m_nodeTail;
        return m_nodeTail->m_tItem;
    }

    //================================================================
    //向后移动当前节点指针，并返回当前位置节点内容
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
    //向前移动当前节点指针，并返回当前位置节点内容
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
    //插入另外一个双向不循环链表的指定节点以后的所有节点，返回值是本链
    //表节点的总个数
    int Push(CTDChain<T, sort, equal, pointer> *&chain)
    {
        m_nNodeNum += m_nodeTail->CTDChain<T, sort, equal, pointer>::Merge(chain);
        m_nodeHead = m_nodeHead->Head();
        m_nodeTail = m_nodeTail->Tail();
        return m_nNodeNum;
    }

    //================================================================
    // 插入另外一个链表管理类的所有节点，返回值是本链表节点的总个数
    int Push(CTDChainMgr *chainmgr)
    {
        if (!chainmgr || !chainmgr->m_nNodeNum)
            return 0;
        Push(chainmgr->m_nodeHead);
        chainmgr->Empty(); //
        return m_nNodeNum;
    }

    //================================================================
    // 插入一个节点，返回值指示插入是否成功
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
    // 搜索一个节点
    int IsInChain(T &item)
    {
        return (m_nodeCurr->Search(item) == NULL) ? 0 : 1;
    }

    //================================================================
    //排序
    void Sort()
    {
        m_nodeHead->Sort();
        m_nodeHead = m_nodeHead->Head();
        m_nodeTail = m_nodeTail->Tail();
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////双向循环链表模板管理类//////////////////////////////////////////////////
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
    //置当前节点指针(m_nodeCurr)为链表头
    T Begin()
    {
        m_nodeCurr = m_nodeHead;
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    //向后移动当前节点指针，并返回前一位置节点内容
    T Next()
    {
        m_nodeCurr = m_nodeCurr->m_pNext;
        if (m_nodeCurr == m_nodeHead)
            return NULL;
        return m_nodeCurr->m_tItem;
    }

    //================================================================
    // 插入一个节点
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
    // 去掉一个节点，释放这个节点的空间
    void Pop(T &item)
    {
        CTDLChain<T, sort, equal, pointer> *node = m_nodeHead->Search(item);

        if (node == m_nodeHead)
            m_nodeHead = m_nodeHead->m_pNext;
        delete node;
        m_nNodeNum--;
    }

    //================================================================
    // 搜索一个节点
    int IsInChain(T &item)
    {
        return (NULL == m_nodeCurr->Search(item)) ? 0 : 1;
    }

    //================================================================
    //排序
    void Sort()
    {
        m_nodeHead->Sort();
    }
};

#endif //_DoubleDirectChainTemplateClass_H
