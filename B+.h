#include <cstddef>
#include <iostream>
using namespace std;

template<typename data_type, typename key_type, typename getKey, int order = 4, int L = 4>
class B_Tree
{
public:
    B_Tree(): root(nullptr)
    {
        init();
    }
private:
    void init()
    {
        root = new Node( NODE );
        root->tag = NODE;
        root->count = 1;
        root->key[0] = key_type(8);
        
        // left
        root->branch.node[0] = new Node( LEAF );
        Node* left = root->branch.node[0];
        left->count = 2;
        left->key[0] = 2;
        left->key[1] = 4;
        left->branch.leaf[0] = new Leaf( data_type(0), data_type(1) );
        left->branch.leaf[1] = new Leaf( data_type(2), data_type(3) );
        left->branch.leaf[2] = new Leaf( data_type(4), data_type(5) );

        //right
        root->branch.node[1] = new Node( LEAF );
        Node* right = root->branch.node[1];
        right->count = 2;
        right->key[0] = 11;
        right->key[1] = 27;
        right->branch.leaf[0] = new Leaf( data_type(8), data_type(9) );
        right->branch.leaf[1] = new Leaf( data_type(11), data_type(12) );
        right->branch.leaf[2] = new Leaf( data_type(27), data_type(28) );
    }

private:
    enum State { success, failure, overflow, duplicate };
    
    struct Leaf
    {
        Leaf( data_type a, data_type b )
        {
            data[0] = a;
            data[1] = b;
            count = 2;
        }
        Leaf() {}
        int count;
        data_type data[L];
    };

    enum NodeTag:char { LEAF, NODE };
    struct Node
    {
        int count;
        union Branch
        {
            Node* node[order];
            Leaf* leaf[order];
        };
        Branch branch;
        key_type key[order-1];
        NodeTag tag;
        Node( NodeTag t = NODE ): tag(t) {}
    };
    
    union ptr
    {
        Node* node;
        Leaf* leaf; 
    }; //一个可以是叶子又可以是内点的union

    // struct insertNode
    // {}; //用于判断是将数据插入内点还是叶子，无意义
    // struct insertLeaf
    // {}; //用于判断是将数据插入内点还是叶子，无意义

public:
    bool contains( const key_type& x ) const;
    bool insert( const data_type& x );
    void erase( const data_type& y);  
    void display() const
    {
        display(root);
    }

// private:
public:
    Node* root;
    State insert( Node*& n, const key_type& key, const data_type& data, key_type& newKey, Node*& newBranch);
    State insert( Leaf*& l, const key_type& key, const data_type& data, key_type& newKey, Leaf*& newBranch);
    void insert_key( Node* n, const key_type& newkey, void* newBranch, size_t pos );
    void insert_data( Leaf* l, const data_type& newdata, size_t pos ); 
    size_t findPos( Node* n, const key_type& x ) const;
    size_t findPos( Leaf* l, const data_type& d ) const;
    void split( Node* n, const key_type& cur_newKey, Node* cur_newBranch, size_t pos, key_type& newKey, Node*& newBranch);
    void split( Leaf* l, const data_type& data, size_t pos, key_type& newKey, Leaf*& newBranch );
    void display( Node* n, int indent = 0 ) const;
    void display( Leaf* l, int indent = 0 ) const;


    //删除自定义的privata;
    void erase(Node*& n, const data_type& y);
    void remove_inleaf(Leaf* current, const data_type& x);
    void restore_inleaf(Node* current, const int& position);
    void restore_innode(Node* current, const int& position);
    //还需要一个关注根节点的指针；

    //restore_innode要用到的三个方法；
    void movenode_left(Node* current, const int& position);
    void movenode_right(Node* current, const int& position);
    void movenode_combine(Node* current, const int& position);

    //restore_in
    void moveleaf_left(Node* current, const int& position);
    void moveleaf_right(Node * current, const int& position);
    void moveleaf_combine(Node* current, const int& keyposition);

    void showroot()
    {
        cout<<root -> count <<endl;
        cout<<root -> key[0] <<endl;
    }
    
private:    
    inline void displayIndent( int indent ) const
    {
        for( int i = 0; i < indent; ++i )
            cout << "\t";
    }
};


template<typename data_type, typename key_type, typename getKey, int order, int L>
bool B_Tree<data_type, key_type, getKey, order, L>::insert( const data_type& data )
{
    getKey get;
    key_type key = get( data ); //获取data的关键字
    key_type newKey;
    Node* newBranch;
    State result = insert( root, key, data, newKey, newBranch );
    if( result == overflow ) //以newKey和newBranch创建新根
    {
        Node* newRoot = new Node;
        newRoot->count = 1;
        newRoot->key[0] = newKey;
        newRoot->branch.node[0] = root;
        newRoot->branch.node[1] = newBranch;
        root = newRoot;
        result = success; 
    }
    if( result == duplicate )
        return false;
    else
        return true;
}   

template<typename data_type, typename key_type, typename getKey, int order, int L>
typename B_Tree<data_type, key_type, getKey, order, L>::State
B_Tree<data_type, key_type, getKey, order, L>::insert( Node*& n, const key_type& key, const data_type& data, key_type& newKey, Node*& newBranch)
{
    size_t pos = findPos( n, key );
    ptr next_node { n->branch.node[pos] };
    
    if( n->tag == LEAF )
    {
        for( int i = 0; i < next_node.leaf->count; ++i )
            if( next_node.leaf->data[i] == data )
                return duplicate;
    }
        
    key_type cur_newKey;
    ptr cur_newBranch;
    State result;
    if( n->tag == NODE )
        result = insert( next_node.node, key, data, cur_newKey, cur_newBranch.node );
    else if( n->tag == LEAF )
        result = insert( next_node.leaf, key, data, cur_newKey, cur_newBranch.leaf );
    
    if( result == overflow )
    {
        if( n->count < order - 1 )
        {
            result = success;
            insert_key( n, cur_newKey, cur_newBranch.node, pos );
        }
        else
            split( n, cur_newKey, cur_newBranch.node, pos, newKey, newBranch );
        //需要解决：含叶子与不含叶子是否有本质不同？    
        //通过下一级操作因overflow所产生的新键和新分支来更新newKey和newBranch给上一级使用
    }
    return result;
} 

template<typename data_type, typename key_type, typename getKey, int order, int L>
typename B_Tree<data_type, key_type, getKey, order, L>::State
B_Tree<data_type, key_type, getKey, order, L>::insert( Leaf*& l, const key_type& key, const data_type& data, key_type& newKey, Leaf*& newBranch )
{
    State result = success;
    size_t pos = findPos( l, data );
    if( l->count < L )
        insert_data( l, data, pos );
    else
    {
        split( l, data, pos, newKey, newBranch ); //通过向叶子中插入data 产生新的key和新的branch 
        result = overflow;
    }
    return result;
}

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::insert_data( Leaf* l, const data_type& newdata, size_t pos ) 
{
    for( int i = l->count; i > pos; --i ) //包括pos在内都向前移位
        l->data[i] = l->data[i-1];
    l->data[pos] = newdata;
    l->count++; 
}

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::insert_key( Node* n, const key_type& newkey, void* newBranch, size_t pos ) 
{
    for( int i = n->count; i > pos; --i ) //包括pos在内都向前移位
    {
        n->key[i] = n->key[i-1];
        n->branch.node[i+1] = n->branch.node[i];
    }
    n->key[pos] = newkey;
    n->branch.node[pos+1] = (Node*)newBranch;   //branch的pos部分已经更新不需更改，只需插入新生成的分支即可
    n->count++; 
}   

template<typename data_type, typename key_type, typename getKey, int order, int L>
size_t B_Tree<data_type, key_type, getKey, order, L>::findPos( Node* n, const key_type& x ) const
{
    for( int i = 0; i < n->count; ++i )
    {
        if( x < n->key[i] )
            return i;
    }
    return n->count;
}   

template<typename data_type, typename key_type, typename getKey, int order, int L>
size_t B_Tree<data_type, key_type, getKey, order, L>::findPos( Leaf* l, const data_type& data ) const
{
    for( int i = 0; i < l->count; ++i )
    {
        if( data < l->data[i] )
            return i;
    }
    return l->count;
}

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::split( Node* n, const key_type& cur_newKey, Node* cur_newBranch, size_t pos, key_type& newKey, Node*& newBranch)
{
    newBranch = new Node( n->tag ); //分出的新结点的tag肯定和n相同
    size_t mid = order / 2;
    if( pos >= mid ) //使mid向后数的关键字个数总小于等于左半边的
        mid++;

    for( int i = mid; i < order-1; ++i ) //拷贝key和branch 
    {
        //注意!!下标为order-2为第order-1个元素
        newBranch->key[i-mid] = n->key[i];
        newBranch->branch.node[i-mid+1] = n->branch.node[i+1];
    } 
    n->count = mid;
    newBranch->count = order - 1 - mid;
    
    if( pos >= mid ) //与mid自不自增无关 位置都是pos-mid 因为拷贝都把mid拷贝过去了 
        insert_key( newBranch, cur_newKey, cur_newBranch, pos - mid );
    else
        insert_key( n, cur_newKey, cur_newBranch, pos );
    
    //注意 插入完才能决定右半部分的branch【0】是什么
    newKey = n->key[n->count - 1];
    newBranch->branch.node[0] = n->branch.node[n->count]; 
    n->count--;   
}   

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::split( Leaf* l, const data_type& data, size_t pos, key_type& newKey, Leaf*& newBranch )
{
    newBranch = new Leaf;
    size_t mid = L / 2;
    if( pos >= mid ) //使mid向后数的关键字个数总小于等于左半边的 
        mid++;  //等号不可以去掉！！(为何？？)

    for( int i = mid; i < L; ++i ) //拷贝data和branch 
        newBranch->data[i-mid] = l->data[i];
    
    l->count = mid;
    newBranch->count = L - mid;
    
    if( pos >= mid ) //与mid自不自增无关 位置都是pos-mid 因为拷贝都把mid拷贝过去了 
        insert_data( newBranch, data, pos - mid );
    else
        insert_data( l, data, pos );
    
    getKey get;
    newKey = get(newBranch->data[0]); //新结点第一个data的关键字作为新键
}   

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::display( Node* n, int indent ) const
{
    if( n->tag == NODE )
    {
        for( int i = n->count - 1; i >= 0; --i )
        {
            display( n->branch.node[i+1], indent + 1 );
            displayIndent(indent);
            cout << n->key[i] << endl;
        }
        display( n->branch.node[0], indent + 1 );
    }
    else if( n->tag == LEAF )
    {
        for( int i = n->count - 1; i >= 0; --i )
        {
            display( n->branch.leaf[i+1], indent + 1 );
            displayIndent(indent);
            cout << n->key[i] << endl;
        }
        display( n->branch.leaf[0], indent + 1 );
    }       
} 

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::display( Leaf* l, int indent ) const
{
    displayIndent(indent);
    
    for( int i = 0; i < l->count; ++i )
        cout << l->data[i] << ' ';

    cout << endl;
}

























//专门用于删除叶节点的数据，但是这里不删除节点；
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::remove_inleaf(Leaf* current, const data_type& y)
{
    for(int i = 0; i<current -> count; i++)
        if(current -> data[i] == y)
        {
            current -> count--;
            while(i<current -> count)
            {
                current -> data[i] = current -> data[i+1];
                ++i;
            }
            break;
        }
}

//该函数处理含有叶结点指针的节点的左旋转
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::moveleaf_left(Node* current, const int& position)
{
    getKey getkey;
    Leaf* right = current -> branch.leaf[position+1];
    Leaf* left = current -> branch.leaf[position];
    current -> key[position] = getkey(right -> data[1]);
    data_type temp = right -> data[0];
    right -> count--;
    for(int i = 0; i<right->count; i++)
        right -> data[i] = right -> data[i+1];
    int& tempcount = left -> count;
    left -> data[tempcount++] = temp;
}

//该函数处理含有叶结点指针的节点的右旋转
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::moveleaf_right(Node* current, const int& position)
{
    getKey getkey;
    Leaf* left = current -> branch.leaf[position-1];
    Leaf* right = current -> branch.leaf[position];
    current -> key[position-1] = getkey(left -> data[1]);
    data_type temp = left -> data[0];
    left -> count--;
    for(int i = 0; i<left->count; i++)
        left -> data[i] = left -> data[i+1];
    int& tempcount = right -> count;
    right -> data[tempcount++] = temp;
}

//合并，我选择右边合到左边，这样少1步；
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::moveleaf_combine(Node* current, const int& keyposition)
{
    //cout<<"moveleaf_combine"<<endl;
    Leaf* left = current -> branch.leaf[keyposition];
    Leaf* right = current -> branch.leaf[keyposition+1];
    int temp = left -> count;
    left -> count += right -> count;
    //cout<<"hello"<<endl;
    for(int i = temp; i<left -> count; i++)
    {
        left -> data[i] = right -> data[i-temp];//将右边的叶子的数据都移动到左边的叶子;
    }
    delete right;
    for(int i = keyposition+1; i<current -> count; i++)//更新叶子指针数组；
        current -> branch.leaf[i] = current -> branch.leaf[i+1];
    current -> count--;//更新count;
    for(int i = keyposition; i<current -> count; i++)//更新键值数组；
        current -> key[i] = current -> key[i+1];
    //cout<<"hello"<<endl;
    //cout<<current -> count<<endl;
}

//对带有叶节点指针的node节点的调整
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::restore_inleaf(Node* current, const int& position)
{
    //cout<<position;
    if(position == 0)
    {
        if(current -> branch.leaf[position+1] -> count > (L-1)/2)
            moveleaf_left(current, position);
        else
            moveleaf_combine(current, 0);
    }
    else if(position == current-> count)
    {
        if(current -> branch.leaf[position-1] -> count > (L-1)/2)
            moveleaf_right(current, position);
        else
            moveleaf_combine(current, position - 1);
    }
    else
    {
        if(current -> branch.leaf[position+1] -> count > (L-1)/2)
        {
            moveleaf_left(current, position);
        }
        else if(current -> branch.leaf[position-1] -> count > (L-1)/2)
        {
            moveleaf_right(current, position);
        }
        else
        {
            moveleaf_combine(current, position);
            //cout<<"hello"<<endl;
        }
    }
}

//将右边元素放到左边
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::movenode_left(Node* current, const int& position)
{
    Node* right = current -> branch.node[position+1];
    Node* left = current -> branch.node[position];
    key_type temp = current -> key[position];
    current -> key[position] = right -> key[0];
    Node* temp1 = right -> branch.node[0];
    right -> count--;
    for(int i=  0; i<right -> count; i++)
    {
        right -> key[i] = right -> key[i+1];
        right -> branch.node[i] = right -> branch.node[i+1];
    }
    right -> branch.node[right -> count] = right -> branch.node[right -> count+1];
    left -> key[left -> count++] = temp;
    left -> branch.node[left -> count] = temp1;
}

//将左边元素放到右边
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::movenode_right(Node* current, const int& position)
{
    Node* left = current -> branch.node[position-1];
    Node* right = current -> branch.node[position];
    key_type temp = current -> key[position-1];
    current -> key[position-1] = left -> key[0];
    Node* temp1 = left -> branch.node[0];
    left -> count--;
    for(int i = 0; i<left -> count; i++)
    {
        left -> key[i] = left -> key[i+1];
        left -> branch.node[i] = left -> branch.node[i+1];
    }
    left -> branch.node[left -> count] = left -> branch.node[left -> count +1];
    right -> key[right -> count++] = temp;
    right -> branch.node[right -> count] = temp1;
}

//虽然用的for比较多，但是实际上循环次数不多；这里的参数keyposition代表关键词下标，需要注意；
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::movenode_combine(Node* current, const int& keyposition)
{
    Node* left = current -> branch.node[keyposition];
    Node* right = current -> branch.node[keyposition+1];
    if(current -> count == 1 && current == root)
    {
        current -> tag = left -> tag;
        int temp_count = left -> count;
        for(int i = 0; i<temp_count+1; i++)
        current -> branch.node[keyposition + i] = left -> branch.node[i];
        for(int i = 0; i<right -> count+1; i++)
        current -> branch.node[keyposition + temp_count + i + 1] = right -> branch.node[i];
        current -> key[keyposition + temp_count] = current -> key[keyposition];
        for(int i = 0; i<temp_count; i++)
        current -> key[keyposition + i] = left -> key[i];
        for(int i = 0; i<right -> count; i++)
        current -> key[keyposition + temp_count +i +1] = right -> key[i];
        current -> count = temp_count + current -> count + right -> count;
        delete left;
        delete right;
    }
    else
    {
        int temp_count = left -> count+1;
        key_type temp_key = current -> key[keyposition];
        current -> count -= 1;
        for(int i = keyposition; i< current -> count; i++)
        {
            current -> key[i] = current -> key[i+1];
            current -> branch.node[i+1] = current -> branch.node[i+2];
        }
        left -> key[left -> count] = temp_key;
        left -> count += right -> count +1;
        for(int i = temp_count; i< left-> count; i++)
        {
            left -> key[i] = right -> key[i-temp_count];
            left -> branch.node[i] = right -> branch.node[i-temp_count];
        }
        left -> branch.node[left-> count] = right -> branch.node[right-> count];
        delete right;
    }
}

//每一层视情况决定是否调用这个函数，该调整针对子节点不是叶子节点的情；
template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::restore_innode(Node* current, const int& position)
{
    if(position == 0)
    {
        if(current -> branch.node[position +1] -> count > (order-1)/2)
            movenode_left(current, position);
        else
            movenode_combine(current, 0);
    }
    else if(position == current -> count)
    {
        if(current -> branch.node[position -1] -> count > (order-1)/2)
            movenode_right(current, position);
        else
            movenode_combine(current, position -1);
    }
    else
    {
        if(current -> branch.node[position +1] -> count > (order-1)/2)
            movenode_left(current, position);
        else if(current -> branch.node[position -1] -> count > (order -1)/2)
            movenode_right(current, position);
        else
            movenode_combine(current, position);
    }
}

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::erase( const data_type& y)
{
    erase(root, y);
}

template<typename data_type, typename key_type, typename getKey, int order, int L>
void B_Tree<data_type, key_type, getKey, order, L>::erase(Node*& current, const data_type& y)
{
    getKey getkey;
    key_type x = getkey(y);
    if(current -> tag == LEAF)
    {
        int position = findPos(current, x);
        remove_inleaf(current -> branch.leaf[position],y);
        if(current -> branch.leaf[position] -> count < (L-1)/2)
            restore_inleaf(current, position);
            //这里需要注意，可能需要delete操作；
    }
    else
    {
        int position = findPos(current, x);
        erase(current -> branch.node[position], y);
        // if(current == root)
        //     cout<< current -> branch.node[position] -> count;
        if(current -> branch.node[position] -> count < (order-1)/2)
            restore_innode(current, position);
    }
}
