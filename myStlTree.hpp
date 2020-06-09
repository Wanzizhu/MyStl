#include <memory>

// 代码写完后调整的时间主要用于调整三个函数，increment,insert_unique和rebalance
// 主要的问题有
// 1. 类型转换，由_rb_tree_node_base*转为_rb_tree_node，
// 2. 函数中变量在后续应用中失效了，在后面的代码中不能再使用这些失效的变量，比如在_rb_tree_rebalance中的p
// 3. 如何处理++，--的临界情况，主要是在increment和decrement这两个文件中
// 4. insert_unique确实没怎么懂

enum _rb_tree_color_type
{
    _red = false,
    _black = true
};

struct _rb_tree_node_base
{
    typedef _rb_tree_node_base *base_ptr;
    typedef _rb_tree_color_type color_type;

    base_ptr parent, left, right;
    color_type color;

    // minimum 和maximum设置为static是为了后续应用方便，可以直接用类引用
    //在源码中加入了形参为const base_ptr 的重载函数
    static base_ptr minimum(base_ptr x)
    {
        while (x->left)
        {
            x = x->left;
        }
        return x;
    }
    static base_ptr maximum(base_ptr x)
    {
        while (x->right)
        {
            x = x->right;
        }
        return x;
    };
};

template <typename T>
struct _rb_tree_node : public _rb_tree_node_base
{
    typedef _rb_tree_node<T> *link_type;
    T value;
};

struct _rb_tree_base_iterator
{
    _rb_tree_node_base *node;

    //这里第一次写写错了，注意考虑最右边节点++的情况
    void increment()
    {
        if (node->right != nullptr)
            node = _rb_tree_node_base::minimum(node->right);
        else
        {
            while (node == node->parent->right)
            {
                node = node->parent;
            }
            // when node==header，node—>right==node->parent,这是用于处理只有一个节点的情况。
            if (node->right != node->parent)
                node = node->parent;
        }
    }

    //如何处理最左边节点--的情况
    void decrement()
    {
        // 以下情况发生于node是header的情况，header的前一个是整棵树的最大值，而header.right=rightmost()
        if (node->color == _red && node->parent->parent == node)
            node = node->right;
        else if (node->left != nullptr)
            node = _rb_tree_node_base::maximum(node->left);
        else
        {
            while (node == node->parent->left)
            {
                node = node->parent;
            }
            node = node->parent;
        }
    }
};

template <typename T>
struct _rb_tree_iterator : public _rb_tree_base_iterator
{
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

    typedef _rb_tree_node<T> *link_type;
    typedef _rb_tree_iterator<T> self;

    _rb_tree_iterator(){};
    // 第一次写时把node=x写成了初始化列表的形式，:node(x)，这样写报错了，因为这样写要求node是_rb_tree_iterator中定义的成员
    _rb_tree_iterator(link_type x) { node = x; }
    _rb_tree_iterator(const self &rth) { node = rth.node; }

    self &operator++()
    {
        increment();
        return *this;
    }
    self operator++(int)
    {
        self tmp(*this);
        increment();
        return tmp;
    }

    self &operator--()
    {
        decrement();
        return *this;
    }
    self operator--(int)
    {
        self tmp(*this);
        decrement();
        return tmp;
    }

    reference operator*() { return ((link_type)(node))->value; }
    pointer operator&() { return &(operator*()); }
    bool operator==(const self &rth) const { return node == rth.node; }
    bool operator!=(const self &rth) const { return node != rth.node; }
};

// KeyOfValue :get key from value
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
class rb_tree
{
protected:
    typedef _rb_tree_node_base *base_ptr;
    typedef _rb_tree_node<Value> rb_tree_node;
    typedef _rb_tree_color_type color_type;

public:
    typedef Key key_type;
    typedef Value value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef rb_tree_node *link_type;
    typedef rb_tree<Key, Value, KeyOfValue, Compare> self;

protected:
    std::allocator<rb_tree_node> alloc;
    link_type get_node() { return alloc.allocate(1); }
    void put_node(link_type p) { alloc.deallocate(p, 1); }
    link_type create_node(const value_type &val)
    {
        link_type node = get_node();
        alloc.construct(&node->value, val);
        return node;
    }
    void destory_node(link_type p)
    {
        alloc.destroy(&p->value);
        put_node(p);
    }
    // 为什么在源代码中只指定了tmp的left，right，没有指定parent,而且源代码中参数是传值的
    link_type clone_node(const link_type &node)
    {
        link_type tmp = create_node(node->value);
        tmp->color = node->color;
        tmp->left = tmp->right = nullptr;
        return tmp;
    }

protected:
    link_type header;
    size_type node_count;
    Compare key_compare;

    // 这里需要类型转换,书上是这么写的，源码中没有返回引用，使用了static_cast<link_type>
    link_type &root() const { return (link_type &)(header->parent); }
    link_type &leftmost() const { return (link_type &)(header->left); }
    link_type &rightmost() const { return (link_type &)(header->right); }

    static link_type &left(link_type x) { return (link_type &)(x->left); }
    static link_type &right(link_type x) { return (link_type &)(x->right); }
    static link_type &parent(link_type x) { return (link_type &)(x->parent); }
    static key_type &key(link_type x) { return KeyOfValue()(x->value); }
    static reference value(link_type x) { return x->value; }
    static color_type &color(link_type x) { return (color_type &)(x->color); }

    static link_type minimum(link_type x) { return (link_type)_rb_tree_node_base::minimum(x); }
    static link_type maximum(link_type x) { return (link_type)_rb_tree_node_base::maximum(x); }

public:
    typedef _rb_tree_iterator<value_type> iterator;
    typedef const iterator const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    template <typename InputIterator>
    using RequireInputIterator = typename std::enable_if<std::is_convertible<typename std::iterator_traits<InputIterator>::iterator_category,
                                                                             std::input_iterator_tag>::value>::type;

    base_ptr rebalance_for_erase(base_ptr z, base_ptr h);
    void erase_aux(iterator pos);
    link_type M_copy(const link_type &node, const base_ptr &p);
    link_type M_copy(const rb_tree &rth);
    void M_reset()
    {
        header->parent = nullptr;
        header->left = header->right = header;
        node_count = 0;
    }
    void M_erase(link_type x);
    iterator insert_aux(base_ptr x, base_ptr y, const value_type &val);
    void _rb_tree_rotate_left(base_ptr x, base_ptr &root);
    void _rb_tree_rotate_right(base_ptr x, base_ptr &root);
    void _rb_tree_rebalance(base_ptr x, base_ptr &root);
    void init()
    {
        header = get_node();
        header->color = _red;
        header->parent = nullptr;
        header->left = header->right = header;
    }

public:
    rb_tree(const Compare &cmp = Compare()) : node_count(0), key_compare(cmp) { init(); }
    rb_tree(const rb_tree &rth) : key_compare(rth.key_compare)
    {
        init();
        root() = M_copy(rth);
    };
    self &operator=(const self &rth)
    {
        if (this != &rth)
        {
            clear();
            key_compare = rth.key_compare;
            if (rth.root() != nullptr)
                root() = M_copy(rth);
        }
        return *this;
    };
    ~rb_tree() { clear(); }

    Compare key_comp() const { return key_compare; }
    iterator begin() { return leftmost(); }
    iterator begin() const { return leftmost(); }
    const_iterator cbegin() const { return leftmost(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    iterator end() { return header; }
    iterator end() const { return header; }
    const_iterator cend() const { return header; }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    bool empty() const { return node_count == 0; }
    size_type size() const { return node_count; }
    size_type max_size() const { return size_type(-1); }

    std::pair<iterator, bool> insert_unique(const value_type &val);
    iterator insert_unique(iterator pos, const value_type &val);
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    void insert_unique(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
            insert_unique(end(), *first);
    };

    iterator insert_equal(const value_type &val);
    iterator insert_equal(iterator pos, const value_type &val);
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    void insert_equal(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
            insert_equal(end(), *first);
    };

    // remove all element that element.key ==k
    iterator erase(iterator pos);
    void erase(iterator first, iterator last);
    size_type erase(const key_type &k);
    void clear()
    {
        M_erase(root());
        M_reset();
    }

    // return the first element position that key >k;
    iterator upper_bound(const key_type &k)
    {
        iterator iter = begin();
        while (iter != end() && !key_compare(k, key((link_type)(iter.node))))
        {
            ++iter;
        }
        return iter;
    };

    // return the first element position that key>=k
    iterator lower_bound(const key_type &k)
    {
        iterator iter = begin();
        while (iter != end() && key_compare(key((link_type)(iter.node)), k))
        {
            ++iter;
        }
        return iter;
    };

    std::pair<iterator, iterator> equal_range(const key_type &k)
    {
        return std::make_pair(lower_bound(k), upper_bound(k));
    }
    iterator find(const key_type &k);
};
