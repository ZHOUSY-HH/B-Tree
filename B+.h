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
    

private:
    Node* root;
};


#endif