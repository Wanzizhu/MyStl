#include <iostream>
#include <initializer_list>
#include <memory>

template <typename T>
class MyVector
{
public:
    typedef T value_type;
    typedef value_type *pointer;
    typedef value_type *iterator;
    typedef value_type &reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::reverse_iterator<iterator> reverse_iterator;

private:
    iterator start;
    iterator finish;
    iterator end_of_storage;
    std::allocator<T> alloc;

    //关于异常处理的书写还不太熟练
    template <typename _ForwardIterator>
    iterator allocate_and_copy(size_type n, _ForwardIterator first, _ForwardIterator last)
    {
        iterator result = alloc.allocate(n);
        try
        {
            std::uninitialized_copy(first, last, result);
        }
        catch (...)
        {
            alloc.deallocate(result, n);
        }
        return result;
    }

    void fill_initialize(size_type n, const T &val)
    {
        start = alloc.allocate(n);
        std::uninitialized_fill_n(start, n, val);
        end_of_storage = finish = start + n;
    }

    void destory()
    {
        for (auto iter = start; iter != finish; ++iter)
        {
            alloc.destroy(iter);
        }
        alloc.deallocate(start, end_of_storage - start);
    }

    template <typename InputIterator>
    void assign_auc(InputIterator first, InputIterator last);
    iterator insert_aux(iterator pos, const value_type &val);

public:
    //constructor and destructor
    MyVector() : start(nullptr), finish(nullptr), end_of_storage(nullptr){};
    explicit MyVector(size_type n) { fill_initialize(n, T()); }
    MyVector(size_type n, const T &val) { fill_initialize(n, val); }
    MyVector(const MyVector<T> &rth);
    //move constructor
    MyVector(MyVector<T> &&rv);
    MyVector(std::initializer_list<T> rth);
    template <typename _InputIterator>
    MyVector(_InputIterator first, _InputIterator last);
    ~MyVector() { destory(); }

    MyVector<T> &operator=(const MyVector<T> &rth);

    iterator begin() { return start; }
    const iterator begin() const { return start; }
    iterator end() { return finish; }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    size_type size() { return size_type(end() - begin()); }
    size_type max_size() { return size_type(end_of_storage - start); }
    size_type capacity() { return size_type(end_of_storage - start); }
    bool empty() { return finish == start; }
    reference operator[](size_type index) { return *(start + index); }

    reference front() { return *start; }
    reference back() { return *(end() - 1); }
    void push_back(const value_type &val)
    {
        if (finish != end_of_storage)
        {
            alloc.construct(finish, val);
            ++finish;
        }
        else
            insert_aux(end(), val);
    }
    void pop_back()
    {
        --finish;
        alloc.destroy(finish);
    }

    iterator insert(iterator pos, const value_type &val);
    void insert(iterator pos, size_type n, const value_type &val);
    void insert(iterator pos, const iterator beg, const iterator end);

    iterator erase(iterator pos);
    iterator erase(const iterator first, const iterator end);
    void clear() { erase(begin(), end()); };

    void resize(size_type num, const value_type &val = T());
    void reserve(size_type num);

    template <typename InputIterator>
    using RequireInputIterator = typename std::enable_if<std::is_convertible<typename std::iterator_traits<InputIterator>::iterator_category,
                                                                             std::input_iterator_tag>::value>::type;

    void assign(size_type n, const value_type &val);
    template <typename InputIterator,
              typename = RequireInputIterator<InputIterator>>
    void assign(InputIterator first, InputIterator last)
    {
        assign_auc(first, last);
    };
};
