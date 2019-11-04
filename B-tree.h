#ifndef BPLUS_H
#define BPLUS_H

template<typename data_type, typename key_type, int order, int L>
class B_Tree
{
private:
    struct Node
    {
        int count;
        enum NodeTag { LEAF, NODE };
        union branch
        {
            Node* node[order];
            Leaf* leaf[order];
        };
        key_type key[order-1];
        NodeTag tag;
        Node();
    };
    struct Leaf
    {
        data_type data[L];
    };

public:
    bool find( key_type x ); //查找关键词,返回是否寻找到关键词;
    void insert( key_type x ); //插入操作;
    bool remove( key_type x ); //删除操作，返回是否删除成功; 
    void clear(); //清空树;

private:
    Node* root;
};


#endif
