#include "myVector.hpp"
#include <algorithm>

//下面这两个函数没有考虑性能，而且是否可以变成对一个函数的调用
template <typename T>
MyVector<T>::MyVector(const MyVector<T> &rth)
{
    start = allocate_and_copy(rth.capacity(), rth.begin(), rth.end());
    finish = start + rth.size();
    end_of_storage = start + rth.capacity();
};

//uninitialized_move 和 __uninitialized_move_a的用法还不清晰
// template <typename T>
// MyVector<T>::MyVector(MyVector<T> &&rv)
// {
//     start = alloc.allocate(rv.capacity());
// std::__uninitialized_move_a(rv.begin(), rv.end(), start);
//     finish = start + rv.size();
//     end_of_storage = start + rv.capacity();
// };

template <typename T>
MyVector<T>::MyVector(std::initializer_list<T> rth)
{
    start = allocate_and_copy(rth.size(), rth.begin(), rth.end());
    finish = start + rth.size();
    end_of_storage = start + rth.size();
};

template <typename T>
template <typename _InputIterator>
MyVector<T>::MyVector(_InputIterator first, _InputIterator last)
{
    size_type n = std::distance(first, last);
    start = allocate_and_copy(n, first, last);
    finish = start + n;
    end_of_storage = finish;
};

template <typename T>
template <typename InputIterator>
void MyVector<T>::assign_auc(InputIterator first, InputIterator last)
{
    size_type n = std::distance(first, last);
    if (n > capacity())
    {
        destory();
        start = allocate_and_copy(n, first, last);
        finish = start + n;
        end_of_storage = finish;
    }
    else if (n > size())
    {
        InputIterator mid = first;
        std::advance(mid, size());
        finish = std::copy(first, mid, start);
        finish = std::uninitialized_copy(mid, last, finish);
    }
    else
    {
        iterator new_finish = std::copy(first, last, start);
        for (auto iter = new_finish; iter != finish; ++iter)
            alloc.destroy(iter);
        finish = new_finish;
    }
};

template <typename T>
void MyVector<T>::assign(size_type n, const value_type &val)
{
    if (n > capacity())
    {
        destory();
        fill_initialize(n, val);
    }
    else if (n > size())
    {
        std::fill_n(start, size(), val);
        finish = std::uninitialized_fill_n(finish, n - size(), val);
    }
    else
    {
        iterator new_finish = std::fill_n(start, n, val);
        for (auto iter = new_finish; iter != finish; ++iter)
            alloc.destroy(iter);
        finish = new_finish;
    }
};

template <typename T>
typename MyVector<T>::iterator MyVector<T>::insert_aux(iterator pos, const value_type &val)
{
    if (finish != end_of_storage)
    {
        alloc.construct(finish, *(finish - 1));
        ++finish;
        std::copy_backward(pos, finish - 2, finish - 1);
        *pos = val;
        return pos;
    }
    else
    {
        const size_type old_size = size();
        size_type new_size = old_size != 0 ? 2 * old_size : 1;
        iterator new_start = alloc.allocate(new_size);
        iterator new_finish = new_start;

        new_finish = std::uninitialized_copy(start, pos, new_start);
        alloc.construct(new_finish, val);
        iterator ans_iter = new_finish;
        ++new_finish;
        new_finish = std::uninitialized_copy(pos, end_of_storage, new_finish);

        destory();
        start = new_start;
        finish = new_finish;
        end_of_storage = start + new_size;
        return ans_iter;
    }
};

template <typename T>
typename MyVector<T>::iterator MyVector<T>::insert(iterator pos, const value_type &val)
{
    MyVector<T>::iterator iter = insert_aux(pos, val);
    return iter;
};

template <typename T>
void MyVector<T>::insert(iterator pos, size_type n, const value_type &val)
{
    if (n == 0)
        return;
    if (end_of_storage - finish >= n)
    {
        size_type cur_elem = finish - pos;
        if (cur_elem > n)
        {
            std::uninitialized_copy(finish - n, finish, finish);
            iterator old_finish = finish;
            finish += n;
            std::copy_backward(pos, old_finish - n, old_finish);
            std::fill_n(pos, n, val);
        }
        else
        {
            std::uninitialized_fill_n(finish, n - cur_elem, val);
            iterator old_finish = finish;
            finish += (n - cur_elem);
            std::uninitialized_copy(pos, old_finish, finish);
            finish += cur_elem;
            std::fill(pos, old_finish, val);
        }
    }
    else
    {
        size_type old_size = size();
        size_type new_size = old_size + std::max(old_size, n);
        iterator new_start = alloc.allocate(new_size);
        iterator new_finish = new_start;
        new_finish = std::uninitialized_copy(start, pos, new_start);
        new_finish = std::uninitialized_fill_n(new_finish, n, val);
        new_finish = std::uninitialized_copy(pos, finish, new_finish);
        destory();
        start = new_start;
        finish = new_finish;
        end_of_storage = start + new_size;
    }
};

template <typename T>
void MyVector<T>::insert(iterator pos, const iterator beg, const iterator end)
{
    size_type n = end - beg;
    if (end_of_storage - finish >= n)
    {
        size_type cur_elem = finish - pos;
        if (cur_elem > n)
        {
            std::uninitialized_copy(finish - n, finish, finish);
            iterator old_finish = finish;
            finish += n;
            std::copy_backward(pos, old_finish - n, old_finish);
            std::copy(beg, end, pos);
        }
        else
        {
            std::uninitialized_copy(beg + cur_elem, end, finish);
            iterator old_finish = finish;
            finish += (n - cur_elem);
            std::uninitialized_copy(pos, old_finish, finish);
            finish += cur_elem;
            std::copy(beg, beg + cur_elem, pos);
        }
    }
    else
    {
        size_type old_size = size();
        size_type new_size = old_size + std::max(old_size, n);
        iterator new_start = alloc.allocate(new_size);
        iterator new_finish = new_start;
        new_finish = std::uninitialized_copy(start, pos, new_start);
        new_finish = std::uninitialized_copy(beg, end, new_finish);
        new_finish = std::uninitialized_copy(pos, finish, new_finish);
        destory();
        start = new_start;
        finish = new_finish;
        end_of_storage = start + new_size;
    }
};

template <typename T>
typename MyVector<T>::iterator MyVector<T>::erase(iterator pos)
{

    if (pos + 1 != end())
        std::copy(pos + 1, finish, pos);
    --finish;
    alloc.destroy(finish);
    return pos;
}

template <typename T>
typename MyVector<T>::iterator MyVector<T>::erase(const iterator first, const iterator end)
{
    if (end != finish)
        std::copy(end, finish, first);
    size_type n = end - first;
    iterator old_finish = finish;
    finish -= n;
    for (auto iter = finish; iter != old_finish; ++iter)
        alloc.destroy(iter);
    return first;
};

template <typename T>
void MyVector<T>::resize(size_type num, const value_type &val)
{
    size_type old_size = finish - start;
    if (num > old_size)
    {
        for (size_type i = num; i < old_size; ++i)
            push_back(val);
    }
};

template <typename T>
void MyVector<T>::reserve(size_type num)
{
    size_type old_size = finish - start;
    if (num > old_size)
    {
        iterator new_start = alloc.allocate(num);
        iterator new_finish = new_start;
        new_finish = std::uninitialized_copy(start, finish, new_start);
        destory();
        start = new_start;
        finish = new_finish;
        end_of_storage = start + num;
    }
};

template <typename T>
MyVector<T> &MyVector<T>::operator=(const MyVector<T> &rth)
{
    if (this != &rth)
    {
        destory();
        start = allocate_and_copy(rth.capacity(), rth.begin(), rth.end());
        finish = start + rth.size();
        end_of_storage = start + rth.capacity();
    }
    return *this;
};

//下面两个是非成员函数
template <typename T>
bool operator==(const MyVector<T> &lth, const MyVector<T> &rth)
{
    auto iter1 = lth.begin(), iter2 = rth.begin();
    while (iter1 != lth.end() && iter2 != rth.end())
    {
        if (*iter1 != *iter2)
            break;
        else
        {
            ++iter1;
            ++iter2;
        }
    }
    if (iter1 == lth.end() && iter2 == rth.end())
        return true;
    else
        return false;
};

template <typename T>
bool operator!=(const MyVector<T> &lth, const MyVector<T> &rth)
{
    return !(lth == rth);
}

int main()
{
    MyVector<int> array{4, 5};
    MyVector<int> vet{1, 2, 3, 7, 8};

    vet.assign(array.begin(), array.end());
    vet.assign(3, 5);
    for (auto elem : vet)
        std::cout << elem;
    std::cout << std::endl;
}