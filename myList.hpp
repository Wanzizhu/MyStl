#include <memory>
#include <initializer_list>

//这里的destructor怎么写
template <typename T>
struct _list_node
{
    T data;
    _list_node<T> *prev;
    _list_node<T> *next;
};

template <typename T>
struct _list_iterator
{
    typedef _list_iterator<T> self;
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef _list_node<T> *link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

    link_type node;

    _list_iterator(link_type x = nullptr) : node(x){};
    _list_iterator(const self &rth) : node(rth.node){};
    self &operator=(const self &rth)
    {
        node = rth.node;
        return *this;
    };
    reference operator*() const { return node->data; }
    pointer operator->() const { return &(operator*()); }

    self &operator++()
    {
        node = node->next;
        //开始写return node报错了，报错信息为cannot bind non-const lvalue reference of type self& to an rvalue of type ,因为这里返回self&，所以必须绑定在lvalue，
        //which is a named variable,但是如果返回node，node被自动转换为临时的iterator对象，是不符合要求的
        return *this;
    }
    self operator++(int)
    {
        self tmp(*this);
        ++*this;
        return tmp;
    };
    self &operator--()
    {
        node = node->prev;
        return *this;
    }
    self operator--(int)
    {
        self tmp(*this);
        --*this;
        return tmp;
    }

    bool operator==(const self &rth) const { return rth.node == node; }
    bool operator!=(const self &rth) const { return rth.node != node; }
};

template <typename T>
class MyList
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _list_node<T> list_node;
    typedef list_node *link_type;
    typedef _list_iterator<T> iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

private:
    template <typename InputIterator>
    using RequireInputIterator = typename std::enable_if<std::is_convertible<typename std::iterator_traits<InputIterator>::iterator_category,
                                                                             std::input_iterator_tag>::value>::type;

    std::allocator<list_node> alloc;
    link_type node;
    link_type get_node() { return alloc.allocate(1); }
    void put_node(link_type p) { alloc.deallocate(p, 1); }
    link_type create_node(const T &val)
    {
        link_type p = get_node();
        alloc.construct(&p->data, val);
        return p;
    }
    void destory_node(link_type p)
    {
        alloc.destroy(&p->data);
        put_node(p);
    }
    void empty_initialize()
    {
        node = get_node();
        node->next = node;
        node->prev = node;
    }
    void fill_n_initialize(size_type n, const T &val);
    template <typename InputIterator>
    void fill_initialize(InputIterator first, InputIterator last);
    template <typename InputIterator>
    void insert_auc(iterator pos, InputIterator first, InputIterator last);

    void transfer(iterator pos, iterator first, iterator last);
    void destory()
    {
        link_type cur = node->next;
        while (cur != node)
        {
            link_type tmp = cur;
            cur = cur->next;
            destory_node(tmp);
        }
        node->next = node;
        node->prev = node;
    }

public:
    MyList() { empty_initialize(); }
    MyList(const MyList<T> &rth)
    {
        empty_initialize();
        fill_initialize(rth.begin(), rth.end());
    }
    MyList(size_type n, const T &val = T())
    {
        empty_initialize();
        fill_n_initialize(n, val);
    };
    MyList(std::initializer_list<T> lst)
    {
        empty_initialize();
        fill_initialize(lst.begin(), lst.end());
    }
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    MyList(InputIterator first, InputIterator last)
    {
        empty_initialize();
        fill_initialize(first, last);
    };
    ~MyList() { destory(); }

    MyList<T> &operator=(const MyList<T> &rth)
    {
        destory();
        fill_initialize(rth.begin(), rth.end());
        return *this;
    }

    bool empty() const { return node->next == node; }
    size_type size() const
    {
        size_type result = std::distance(begin(), end());
        return result;
    }
    iterator begin() { return node->next; }
    const iterator begin() const { return node->next; }
    iterator rbegin() { return reverse_iterator(end()); }

    iterator end() { return node; }
    const iterator end() const { return node; }
    iterator rend() { return reverse_iterator(begin()); }

    T &front() { return node->next->data; }
    T &back() { return node->prev->data; }

    iterator insert(iterator pos, const T &val);
    void insert(iterator pos, size_type n, const T &val);
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    void insert(iterator pos, InputIterator first, InputIterator last) { insert_auc(pos, first, last); };

    void push_front(const T &val) { insert(begin(), val); }
    void push_back(const T &val) { insert(end(), val); }
    iterator erase(iterator pos);
    void pop_front() { erase(begin()); }
    void pop_back()
    {
        iterator tmp = end();
        --tmp;
        erase(tmp);
    };

    void clear() { destory(); }

    void remove(const T &val);
    void unique();

    void splice(iterator pos, MyList<T> &lst);
    void splice(iterator pos, MyList<T> &lst, iterator i);
    void splice(iterator pos, MyList<T> &lst, iterator first, iterator last);
    void reverse();
    void merge(MyList<T> &lst);
    void swap(MyList<T> &lst);
    void bubble_sort();
    void sort();
};