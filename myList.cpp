#include <iostream>
#include <algorithm>
#include "myList.hpp"

template <typename T>
void MyList<T>::fill_n_initialize(size_type n, const T &val)
{
    for (int i = 0; i < n; ++i)
        push_back(val);
};

template <typename T>
template <typename InputIterator>
void MyList<T>::fill_initialize(InputIterator first, InputIterator last)
{
    for (; first != last; ++first)
        push_back(*first);
};

template <typename T>
typename MyList<T>::iterator MyList<T>::insert(iterator pos, const T &val)
{
    link_type new_node = create_node(val);
    link_type pre = pos.node->prev;
    pre->next = new_node;
    new_node->prev = pre;
    new_node->next = pos.node;
    pos.node->prev = new_node;
    return new_node;
};

template <typename T>
typename MyList<T>::iterator MyList<T>::erase(iterator pos)
{
    link_type pre = pos.node->prev;
    link_type next = pos.node->next;
    pre->next = next;
    next->prev = pre;
    destory_node(pos.node);
    return iterator(next);
};

template <typename T>
void MyList<T>::remove(const T &val)
{
    iterator iter = begin();
    for (; iter != end();)
    {
        if (*iter == val)
            iter = erase(iter);
        else
            ++iter;
    }
};

template <typename T>
void MyList<T>::unique()
{
    iterator prev = begin();
    iterator last = end();
    if (prev == last)
        return;
    //这里一开始写反了一个错误，没有在循环中更新next。for循环中的第一句next=prev只会在第一次执行
    for (iterator next = prev; ++next != last;)
    {
        if (*next == *prev)
            erase(next);
        else
            prev = next;
        next = prev;
    }
};

template <typename T>
void MyList<T>::transfer(iterator pos, iterator first, iterator last)
{
    first.node->prev->next = last.node;
    last.node->prev->next = pos.node;
    pos.node->prev->next = first.node;
    link_type tmp = first.node->prev;
    first.node->prev = pos.node->prev;
    pos.node->prev = last.node->prev;
    last.node->prev = tmp;
};

template <typename T>
void MyList<T>::splice(iterator pos, MyList<T> &lst)
{
    transfer(pos, lst.begin(), lst.end());
};

template <typename T>
void MyList<T>::splice(iterator pos, MyList<T> &lst, iterator i)
{
    iterator last = i;
    ++last;
    if (pos == i || pos == last)
        return;
    transfer(pos, i, last);
};

template <typename T>
void MyList<T>::splice(iterator pos, MyList<T> &lst, iterator first, iterator last)
{
    if (first == last)
        return;
    transfer(pos, first, last);
};

template <typename T>
void MyList<T>::reverse()
{
    if (node->next == node || node->next->next == node)
        return;
    iterator first = begin();

    //这个是自己写的
    // while (first != end() && first != --end())
    // {
    //     splice(first, *this, --end());
    // }

    //这个是stl源码剖析书上的
    ++first;
    while (first != end())
    {
        iterator old = first;
        ++first;
        transfer(begin(), old, first);
    }
};

template <typename T>
void MyList<T>::merge(MyList<T> &lst)
{
    iterator iter1 = begin();
    iterator iter2 = lst.begin();
    while (iter1 != end() && iter2 != lst.end())
    {
        if (*iter1 <= *iter2)
            ++iter1;
        else
        {
            iterator old = iter2;
            ++iter2;
            transfer(iter1, old, iter2);
        }
    }
    if (iter2 != lst.end())
        transfer(iter1, iter2, lst.end());
};

template <typename T>
void MyList<T>::bubble_sort()
{
    iterator last = end();
    //第一次写犯了一个错误，就是没有在循环里更新start，for条件中第一句只会在第一次执行，
    for (auto start = begin(); start != last;)
    {
        while (start != last)
        {
            iterator next = start;
            ++next;
            if (next != last && *start > *next)
            {
                std::iter_swap(start, next);
            }
            start = next;
        }
        --last;
        start = begin();
    }
};

template <typename T>
void MyList<T>::sort()
{
    int fill = 0;
    MyList<T> carry;
    MyList<T> counter[64];
    while (!empty())
    {
        carry.splice(carry.begin(), *this, begin());
        int i = 0;
        while (i < fill && !counter[i].empty())
        {
            carry.merge(counter[i++]);
        }
        carry.swap(counter[i]);
        if (i == fill)
            ++fill;
    }
    for (int i = 1; i < fill; ++i)
    {
        counter[i].merge(counter[i - 1]);
    }
    swap(counter[fill - 1]);
};

template <typename T>
void MyList<T>::swap(MyList<T> &lst)
{
    MyList<T> tmp(lst);
    lst = *this;
    *this = tmp;
};

int main()
{
    int a[] = {6, 5, 4, 4, 2, 1};
    MyList<int> lst(a, a + 6);
    MyList<int> lst2{5, 4};
    // auto iter = std::distance(lst.begin(), lst.end());
    lst.sort();
    for (auto iter = lst.begin(); iter != lst.end(); ++iter)
        std::cout << *iter << " ";
    std::cout << std::endl;
}