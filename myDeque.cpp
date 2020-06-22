#include "Deque.hpp"
#include <deque>
// 这个函数仅仅将val destroy，并没有进行node的deallocate，以及map的destroy和deallocate
template <typename T>
void MyDeque<T>::destroy(iterator first, iterator last)
{
    for (map_pointer cur = first.node + 1; cur < last.node; ++cur)
    {
        for (size_type i = 0; i < buffer_size(); ++i)
            data_alloc.destroy(*cur + i);
    }
    if (first.node != last.node)
    {
        for (pointer tmp = first.cur; tmp < first.last; ++tmp)
            data_alloc.destroy(tmp);
        for (pointer tmp = last.first; tmp < last.cur; ++tmp)
            data_alloc.destroy(tmp);
    }
    else
    {
        for (pointer tmp = first.cur; tmp < last.cur; ++tmp)
            data_alloc.destroy(tmp);
    }
}

template <typename T>
typename MyDeque<T>::iterator MyDeque<T>::erase(iterator pos)
{
    size_type index = pos - start;
    iterator next = pos;
    ++next;
    if (index < size() / 2)
    {
        std::copy_backward(start, pos, next);
        pop_front();
    }
    else
    {
        std::copy(next, finish, pos);
        pop_back();
    }
    return start + index;
}

template <typename T>
typename MyDeque<T>::iterator MyDeque<T>::erase(iterator first, iterator last)
{
    if (first == start && last == finish)
    {
        clear();
        return finish;
    }
    size_type index = first - start;
    size_type n = last - first;
    if (index < (size() - n) / 2)
    {
        std::copy_backward(start, first, last);
        iterator new_start = start + n;
        for (iterator iter = start; iter < new_start; ++iter)
        {
            data_alloc.destroy(iter.cur);
        }
        for (map_pointer node = start.node; node < new_start.node; ++node)
            deallocate_node(*node);
        start = new_start;
    }
    else
    {
        std::copy(last, finish, first);
        iterator new_finish = finish - n;
        for (iterator iter = new_finish; iter < finish; ++iter)
            data_alloc.destroy(iter.cur);
        for (map_pointer node = new_finish.node + 1; node <= finish.node; ++node)
            deallocate_node(*node);
        finish = new_finish;
    }
    return start + index;
}

template <typename T>
typename MyDeque<T>::iterator
MyDeque<T>::new_elements_at_front(size_type n)
{
    size_type vacancies = start.cur - start.first;
    if (vacancies < n)
    {
        size_type add_nodes = (n - vacancies - 1) / buffer_size() + 1;
        reserve_map_at_front(add_nodes);
        for (size_type i = 1; i <= add_nodes; ++i)
        {
            *(start.node - i) = allocate_node();
        }
    }
    return start - n;
}

template <typename T>
typename MyDeque<T>::iterator
MyDeque<T>::new_elements_at_back(size_type n)
{
    size_type vacancies = finish.last - finish.cur;
    if (vacancies < n)
    {
        size_type add_nodes = (n - vacancies - 1) / buffer_size() + 1;
        reserve_map_at_back(add_nodes);
        for (size_type i = 1; i <= add_nodes; ++i)
        {
            *(finish.node + 1) = allocate_node();
        }
    }
    return finish + n;
}

template <typename T>
template <typename InputIterator>
void MyDeque<T>::insert_aux(iterator pos, InputIterator first, InputIterator last)
{
    difference_type n = std::distance(first, last);
    difference_type elems_before = pos - start;
    // 注意，pos位置上的值是算入elems_after的
    difference_type elems_after = size() - elems_before;
    if (elems_before < size() / 2)
    {
        iterator new_start = new_elements_at_front(n);
        iterator old_start = start;
        pos = old_start + elems_before;
        if (elems_before > n)
        {
            iterator old_start_n = old_start + n;
            std::uninitialized_copy(old_start, old_start_n, new_start);
            std::copy(old_start_n, pos, old_start);
            std::copy_backward(first, last, pos);
        }
        else
        {
            iterator pos1 = std::uninitialized_copy(old_start, pos, new_start);
            InputIterator mid(first);
            std::advance(mid, n - elems_before);
            std::uninitialized_copy(first, mid, pos1);
            std::copy(mid, last, old_start);
        }
        start = new_start;
    }
    else
    {
        iterator new_finish = new_elements_at_back(n);
        iterator old_finish = finish;
        pos = old_finish - elems_after;
        if (elems_after > n)
        {
            iterator old_finish_n = old_finish - n;
            std::uninitialized_copy(old_finish_n, old_finish, old_finish);
            std::copy_backward(pos, old_finish_n, old_finish);
            std::copy(first, last, pos);
        }
        else
        {
            InputIterator mid(first);
            std::advance(mid, elems_after);
            iterator pos1 = std::uninitialized_copy(mid, last, old_finish);
            std::uninitialized_copy(pos, old_finish, pos1);
            std::copy(first, mid, pos);
        }
        finish = new_finish;
    }
}

template <typename T>
typename MyDeque<T>::iterator MyDeque<T>::insert(iterator pos, const value_type &val)
{
    if (pos == start)
        push_front(val);
    else if (pos == finish)
        push_back(val);
    else
    {
        size_type index = pos - start;
        if (index < size() / 2)
        {
            push_front(front());
            iterator f1 = start + 1;
            iterator f2 = f1 + 1;
            pos = start + index;
            iterator pos1 = pos + 1;
            std::copy(f2, pos1, f1);
        }
        else
        {
            push_back(back());
            iterator b1 = finish - 1;
            iterator b2 = b1 - 1;
            std::copy_backward(pos, b2, b1);
        }
        *pos = val;
    }
    return pos;
}

// 注意 deque的初始状态是有一个缓存区，因此最终仍需要保留一个缓存区
template <typename T>
void MyDeque<T>::clear()
{
    destroy(begin(), end());
    for (map_pointer cur = start.node + 1; cur < finish.node + 1; ++cur)
    {
        deallocate_node(*cur);
    }
    finish = start;
}

template <typename T>
typename _deque_iterator<T>::self &_deque_iterator<T>::operator++()
{
    ++cur;
    if (cur == last)
    {
        set_node(node + 1);
        cur = first;
    }
    return *this;
}

template <typename T>
typename _deque_iterator<T>::self _deque_iterator<T>::operator++(int)
{
    self tmp = *this;
    ++*this;
    return tmp;
}

template <typename T>
typename _deque_iterator<T>::self &_deque_iterator<T>::operator--()
{
    if (cur == first)
    {
        set_node(node - 1);
        cur = last;
    }
    --cur;
    return *this;
}

template <typename T>
typename _deque_iterator<T>::self _deque_iterator<T>::operator--(int)
{
    self tmp = *this;
    --*this;
    return tmp;
}

// 第一次写的时候没有写对,这里和 源码有些不一样，之后检查请注意这里
template <typename T>
typename _deque_iterator<T>::self &_deque_iterator<T>::operator+=(difference_type n)
{
    const difference_type off_set = n + cur - first;
    if (off_set >= 0 && off_set < difference_type(buffer_size()))
        cur += n;
    else
    {
        size_type node_off_set = off_set > 0 ? off_set / difference_type(buffer_size()) : -difference_type((-off_set - 1) / buffer_size()) - 1;
        set_node(node + node_off_set);
        cur = first + (off_set - node_off_set * difference_type(buffer_size()));
    }
    return *this;
}

template <typename T>
typename _deque_iterator<T>::self _deque_iterator<T>::operator+(difference_type n) const
{
    self tmp = *this;
    tmp += n;
    return tmp;
}

template <typename T>
typename _deque_iterator<T>::self &_deque_iterator<T>::operator-=(difference_type n)
{
    return *this += -n;
}

template <typename T>
typename _deque_iterator<T>::self _deque_iterator<T>::operator-(difference_type n) const
{
    self tmp = *this;
    return tmp -= n;
}

template <typename T>
void MyDeque<T>::create_map_and_nodes(size_type num_elements)
{
    size_type num_nodes = num_elements / buffer_size() + 1;
    map_size = std::max(initial_map_size, num_nodes + 2);
    map = map_alloc.allocate(map_size);

    map_pointer nstart = map + (map_size - num_nodes) / 2;
    map_pointer nfinish = nstart + num_nodes - 1;
    for (map_pointer i = nstart; i <= nfinish; ++i)
    {
        *i = allocate_node();
    }
    start.set_node(nstart);
    finish.set_node(nfinish);
    start.cur = start.first;
    finish.cur = finish.first + num_elements % buffer_size();
}

template <typename T>
void MyDeque<T>::fill_initialize(size_type n, const value_type &val)
{
    create_map_and_nodes(n);
    // 下面这个是自己一开始写的
    // for (iterator i = start; i != finish; ++i)
    //     *i = val;

    for (map_pointer cur = start.node; cur < finish.node; ++cur)
    {
        std::uninitialized_fill_n(*cur, buffer_size(), val);
    }
    std::uninitialized_fill(finish.first, finish.cur, val);
}

template <typename T>
void MyDeque<T>::push_front(const value_type &val)
{
    if (start.cur != start.first)
    {
        data_alloc.construct(start.cur - 1, val);
        --start.cur;
    }
    else
    {
        push_front_aux(val);
    }
}

template <typename T>
void MyDeque<T>::push_front_aux(const value_type &val)
{
    reserve_map_at_front();
    *(start.node - 1) = allocate_node();
    start.set_node(start.node - 1);
    start.cur = start.last - 1;
    data_alloc.construct(start.cur, val);
}

template <typename T>
void MyDeque<T>::push_back(const value_type &val)
{
    if (finish.cur != finish.last - 1)
    {
        data_alloc.construct(finish.cur, val);
        ++finish.cur;
    }
    else
    {
        push_back_aux(val);
    }
}

template <typename T>
void MyDeque<T>::push_back_aux(const value_type &val)
{
    reserve_map_at_back();
    *(finish.node + 1) = allocate_node();
    data_alloc.construct(finish.cur, val);
    finish.set_node(finish.node + 1);
    finish.cur = finish.first;
}

template <typename T>
void MyDeque<T>::reserve_map_at_front(size_type nodes_to_add)
{
    if (start.node - nodes_to_add < map)
    {
        reserve_map(nodes_to_add, true);
    }
}

template <typename T>
void MyDeque<T>::reserve_map_at_back(size_type nodes_to_add)
{
    if (finish.node + nodes_to_add >= map + map_size)
    {
        reserve_map(nodes_to_add, false);
    }
}

template <typename T>
void MyDeque<T>::reserve_map(size_type nodes_to_add, bool add_at_front)
{
    size_type old_num_nodes = finish.node - start.node + 1;
    size_type new_num_nodes = old_num_nodes + nodes_to_add;
    map_pointer new_nstart;
    if (map_size > 2 * new_num_nodes)
    {
        new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
        if (new_nstart < start.node)
            std::copy(start.node, finish.node + 1, new_nstart);
        else
            std::copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
    }
    else
    {
        size_type new_map_size = map_size + std::max(map_size, new_num_nodes) + 2;
        map_pointer new_map = map_alloc.allocate(new_map_size);
        new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
        std::copy(start.node, finish.node + 1, new_nstart);
        map_alloc.deallocate(map, map_size);
        map = new_map;
        map_size = new_map_size;
    }
    start.set_node(new_nstart);
    finish.set_node(new_nstart + old_num_nodes - 1);
}

template <typename T>
void MyDeque<T>::pop_back()
{
    if (finish.cur != finish.first)
    {
        --finish.cur;
        data_alloc.destroy(finish.cur);
    }
    else
    {
        map_pointer old_finish = finish.node;
        finish.set_node(finish.node - 1);
        finish.cur = finish.last - 1;
        data_alloc.destroy(finish.cur);
        deallocate_node(*old_finish);
    }
}

template <typename T>
void MyDeque<T>::pop_front()
{
    if (start.cur != start.last - 1)
    {
        data_alloc.destroy(start.cur);
        ++start.cur;
    }
    else
    {
        map_pointer old_start = start.node;
        data_alloc.destroy(start.cur);
        start.set_node(start.node + 1);
        start.cur = start.first;
        deallocate_node(*old_start);
    }
}

int main()
{
    std::deque<int> k;
    MyDeque<int> d{1, 2, 3};
    int a[] = {6, 7};
    d.insert(d.begin(), a, a + 2);
    for (auto iter = d.begin(); iter < d.end(); ++iter)
        std::cout << *iter << " ";
    std::cout << std::endl;
}
