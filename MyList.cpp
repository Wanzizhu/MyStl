#include <iostream>
#include <algorithm>
#include "MyList.hpp"

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
void MyList<T>::reverse(){

};

int main()
{
    int a[] = {1, 2, 2, 3, 3, 3};
    MyList<int> lst(a, a + 6);
    // auto iter = std::distance(lst.begin(), lst.end());
    lst.unique();
    for (auto iter = lst.begin(); iter != lst.end(); ++iter)
        std::cout << *iter << " ";
    std::cout << std::endl;
}