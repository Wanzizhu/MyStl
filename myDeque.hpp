#include <iostream>
#include <memory>

const int DEQUE_SIZE = 512;
inline size_t _deque_buf_size(size_t size)
{
    return size < DEQUE_SIZE ? size_t(DEQUE_SIZE) / size : size_t(1);
};

template <typename T>
struct _deque_iterator
{
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;

    typedef T **map_pointer;

    typedef _deque_iterator<T> iterator;
    typedef _deque_iterator self;
    typedef _deque_iterator<const T> const_iterator;

    T *cur;
    T *first;
    T *last;
    map_pointer node;
    // 静态函数和非成员函数不能有cv(const , volatile)限定，否则报错 cannot have cv-qualifier
    static size_type buffer_size()
    {
        return _deque_buf_size(sizeof(T));
    }

    _deque_iterator() : cur(), first(), last(), node()
    {
    }
    _deque_iterator(const self &x) : cur(x.cur), first(x.first), last(x.last), node(x.node) {}
    self &operator=(const self &x)
    {
        cur = x.cur;
        first = x.first;
        last = x.last;
        node = x.node;
        return *this;
    }

    void set_node(map_pointer new_node)
    {
        node = new_node;
        first = *node;
        last = first + (difference_type)(buffer_size());
    }
    reference operator*() { return *cur; }
    pointer operator->() { return &(operator*()); }
    reference operator[](difference_type n) const { return *(*this + n); }
    // buffersize() is a funtion, to do
    difference_type operator-(const self &x) const
    {
        return (node - x.node - 1) * buffer_size() + x.last - x.cur + cur - first;
    }

    self &operator++();
    self operator++(int);
    self &operator--();
    self operator--(int);
    self &operator+=(difference_type n);
    self &operator-=(difference_type n);
    self operator+(difference_type n) const;
    self operator-(difference_type n) const;

    bool operator==(const self &x) const { return cur == x.cur; }
    bool operator!=(const self &x) const { return !(*this == x); }
    bool operator<(const self &x) const { return node == x.node ? cur < x.cur : (node < x.node); }
    bool operator>(const self &x) const { return x < *this; }
    bool operator>=(const self &x) const { return !(*this < x); }
    bool operator<=(const self &x) const { return !(*this > x); }
};

template <typename T>
class MyDeque
{
public:
    typedef T value_type;
    typedef T &reference;
    typedef T *pointer;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _deque_iterator<T> iterator;
    typedef const _deque_iterator<T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    const size_type initial_map_size = 8;

protected:
    static size_type buffer_size()
    {
        return _deque_buf_size(sizeof(T));
    }
    typedef T **map_pointer;

private:
    template <typename InputIterator>
    using RequireInputIterator = typename std::enable_if<std::is_convertible<typename std::iterator_traits<InputIterator>::iterator_category,
                                                                             std::input_iterator_tag>::value>::type;

    std::allocator<T> data_alloc;
    std::allocator<pointer> map_alloc;
    size_type map_size;
    map_pointer map;
    iterator start;
    iterator finish;

private:
    pointer allocate_node() { return data_alloc.allocate(buffer_size()); }
    void deallocate_node(pointer p) { return data_alloc.deallocate(p, buffer_size()); }
    void deallocate_map(map_pointer map, size_type map_size) { map_alloc.deallocate(map, map_size); }
    void create_map_and_nodes(size_type num_elements);
    void destroy(iterator first, iterator last);
    void fill_initialize(size_type n, const value_type &val);
    template <typename InputIterator>
    void range_initialize(InputIterator first, InputIterator last)
    {
        size_type n = std::distance(first, last);
        create_map_and_nodes(n);
        std::uninitialized_copy(first, last, start);
    }

    template <typename InputIterator>
    void insert_aux(iterator pos, InputIterator first, InputIterator last);
    void push_back_aux(const value_type &val);
    void push_front_aux(const value_type &val);
    void reserve_map(size_type nodes_to_add, bool add_at_front);
    iterator new_elements_at_front(size_type n);
    iterator new_elements_at_back(size_type n);
    void reserve_map_at_back(size_type nodes_to_add = 1);
    void reserve_map_at_front(size_type nodes_to_add = 1);

public:
    MyDeque() : map(), start(), finish(), map_size(0) {}
    MyDeque(const MyDeque &x) { range_initialize(x.begin(), x.end()); }
    MyDeque(std::initializer_list<T> l) { range_initialize(l.begin(), l.end()); }
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    MyDeque(InputIterator first, InputIterator last) { range_initialize(first, last); }
    MyDeque(const size_type n, const value_type &val = T()) { fill_initialize(n, val); }
    MyDeque<T> &operator=(const MyDeque &x);
    ~MyDeque<T>()
    {
        destroy(begin(), end());
    };

    iterator begin() { return start; }
    iterator end() { return finish; }
    reference front() { return *start; }
    reference back() { return *(finish - 1); }
    size_type size() const { return finish - start; }
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return finish == start; }
    void clear();

    iterator insert(iterator pos, const value_type &val);
    iterator insert(iterator pos, size_type n, const value_type &val);
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    iterator insert(iterator pos, InputIterator first, InputIterator last)
    {
        difference_type offset = pos - start;
        insert_aux(pos, first, last);
        return start + offset;
    }

    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);

    void push_back(const value_type &val);
    void pop_back();
    void push_front(const value_type &val);
    void pop_front();

    void swap(MyDeque<T> &x)
    {
        std::swap(map, x.map);
        std::swap(start, x.start);
        std::swap(finish, x.finish);
        std::swap(map_size, x.map_size);
    }
};