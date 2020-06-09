#include "myStlTree.hpp"
#include <iostream>

// 下面这个函数是看着源码写的，重点就是怎么判断节点键值重复
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
std::pair<typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator, bool>
rb_tree<Key, Value, KeyOfValue, Compare>::insert_unique(const value_type &val)
{
    link_type y = header;
    link_type node = root();
    bool comp = true;
    while (node != nullptr)
    {
        y = node;
        comp = key_compare(KeyOfValue()(val), key(node));
        node = comp ? left(node) : right(node);
    }
    iterator iter = iterator(y);
    if (comp)
        if (iter == begin())
            return std::pair<iterator, bool>(insert_aux(node, y, val), true);
        else
            --iter;
    if (key_compare(key((link_type)(iter.node)), KeyOfValue()(val)))
        return std::pair<iterator, bool>(insert_aux(node, y, val), true);
    return std::pair<iterator, bool>(iter, false);
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::insert_equal(const value_type &val)
{
    link_type y = header;
    link_type node = root();
    while (node != nullptr)
    {
        y = node;
        if (key_compare(KeyOfValue()(val), key(node)))
            node = left(node);
        else
            node = right(node);
    }
    return insert_aux(node, y, val);
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::insert_aux(base_ptr x_, base_ptr y_, const value_type &val)
{
    link_type x = (link_type)x_;
    link_type y = (link_type)y_;
    link_type tmp = create_node(val);
    left(tmp) = right(tmp) = nullptr;
    parent(tmp) = y;
    if (y == header)
    {
        root() = tmp;
        rightmost() = leftmost() = tmp;
    }
    else if (key_compare(KeyOfValue()(val), key(y)))
    {
        left(y) = tmp;
        if (y == leftmost())
            leftmost() = tmp;
    }
    else
    {
        right(y) = tmp;
        if (y == rightmost())
            rightmost() = tmp;
    }
    _rb_tree_rebalance(tmp, header->parent);
    ++node_count;
    return iterator(tmp);
}

// 下面这两个插入有pos hint的代码的重点在于，如何利用pos，结合insert_aux，完成插入动作
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::insert_equal(iterator pos, const value_type &val)
{
    iterator tmp(pos);
    if (tmp == end())
    {
        // 前提size()>0
        if (size() > 0 && !key_compare(KeyOfValue()(val), key(rightmost())))
            return insert_aux(nullptr, rightmost(), val);
        else
            return insert_equal(val);
    }
    else if (!key_compare(key((link_type)(tmp.node)), KeyOfValue()(val)))
    {
        iterator prev(tmp);
        // 下面的第一个判断处理了临界的--情况。
        if ((link_type)(tmp.node) == leftmost())
            return insert_aux(nullptr, leftmost(), val);
        else if (!key_compare(KeyOfValue()(val), key((link_type)((--prev).node))))
        {
            if (right((link_type)(prev.node)) == nullptr)
                return insert_aux(nullptr, prev.node, val);
            else
                return insert_aux(nullptr, tmp.node, val);
        }
        else
            return insert_equal(val);
    }
    else
    {
        iterator next(tmp);
        // 下面的第一个判断处理了临界的++情况
        if ((link_type)(tmp.node) == rightmost())
            return insert_aux(nullptr, rightmost(), val);
        if (!key_compare(key((link_type)((++next).node)), KeyOfValue()(val)))
        {
            if (right((link_type)(tmp.node)) == nullptr)
                return insert_aux(nullptr, tmp.node, val);
            else
                return insert_aux(nullptr, next.node, val);
        }
        else
            return insert_equal(val);
    }
}

// 如果val和pos.key is equivalent，则返回pos
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::insert_unique(iterator pos, const value_type &val)
{
    iterator tmp(pos);
    if (tmp == end())
    {
        if (size() > 0 && key_compare(key(rightmost()), KeyOfValue()(val)))
            return insert_aux(nullptr, rightmost(), val);
        else
            return insert_unique(val).first;
    }
    else if (key_compare(KeyOfValue()(val), key((link_type)(tmp.node))))
    {
        iterator prev(tmp);
        if ((link_type)(tmp.node) == leftmost())
            return insert_aux(0, leftmost(), val);
        else if (key_compare(key((link_type)((--prev).node)), KeyOfValue()(val)))
        {
            if (right((link_type)(prev.node)) == nullptr)
                return insert_aux(0, prev.node, val);
            else
                return insert_aux(0, tmp.node, val);
        }
        else
            return insert_unique(val).first;
    }
    else if (key_compare(key((link_type)(tmp.node)), KeyOfValue()(val)))
    {
        iterator next(tmp);
        if ((link_type)(tmp.node) == rightmost())
            return insert_aux(0, rightmost(), val);
        else if (key_compare(KeyOfValue()(val), key((link_type)((++next).node))))
        {
            if (right((link_type)(tmp.node)) == nullptr)
                return insert_aux(0, tmp.node, val);
            else
                return insert_aux(0, next.node, val);
        }
        else
            return insert_unique(val).first;
    }
    return pos;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::_rb_tree_rotate_left(base_ptr x, base_ptr &root)
{
    base_ptr right = x->right;
    x->right = right->left;
    if (right->left != nullptr)
    {
        right->left->parent = x;
    }
    right->left = x;
    right->parent = x->parent;
    // 第一次写没有考虑x为root的情况
    if (x == root)
        root = right;
    else if (x == x->parent->left)
        x->parent->left = right;
    else
        x->parent->right = right;
    x->parent = right;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::_rb_tree_rotate_right(base_ptr x, base_ptr &root)
{
    base_ptr left = x->left;
    x->left = left->right;
    if (left->right != nullptr)
        left->right->parent = x;
    left->right = x;
    left->parent = x->parent;
    if (x == root)
        root = left;
    else if (x == x->parent->left)
        x->parent->left = left;
    else
        x->parent->right = left;
    x->parent = left;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::_rb_tree_rebalance(base_ptr x, base_ptr &root)
{
    x->color = _red;
    while (x != root && x->parent->color == _red)
    {
        base_ptr p = x->parent;
        if (p == p->parent->left)
        {
            if (p->parent->right != nullptr && p->parent->right->color == _red)
            {
                base_ptr uncle = p->parent->right;
                p->color = _black;
                uncle->color = _black;
                p->parent->color = _red;
                x = p->parent;
            }
            else
            {
                // 这里需要注意当第一次rotate后，x以及x->parent的指向会变，所有后面不能再用p
                if (x == p->right)
                {
                    x = p;
                    _rb_tree_rotate_left(x, root);
                }
                x->parent->color = _black;
                x->parent->parent->color = _red;
                _rb_tree_rotate_right(x->parent->parent, root);
            }
        }
        else
        {
            if (p->parent->left != nullptr && p->parent->left->color == _red)
            {
                base_ptr uncle = p->parent->left;
                p->color = _black;
                uncle->color = _black;
                p->parent->color = _red;
                x = p->parent;
            }
            else
            {
                // 这里需要注意的点和上面一样
                if (x == p->left)
                {
                    x = p;
                    _rb_tree_rotate_right(x, root);
                }
                x->parent->color = _black;
                x->parent->parent->color = _red;
                _rb_tree_rotate_left(x->parent->parent, root);
            }
        }
    }
    root->color = _black;
};

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::find(const key_type &k)
{
    link_type node = root();
    while (node != nullptr)
    {
        if (key_compare(k, value(node)))
            node = left(node);
        else if (key_compare(value(node), k))
            node = right(node);
        else
            return iterator(node);
    }
    return end();
}

// 删除x为根的子树
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::M_erase(link_type x)
{
    if (x != nullptr)
    {
        M_erase(left(x));
        M_erase(right(x));
        destory_node(x);
    }
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key, Value, KeyOfValue, Compare>::erase(iterator pos)
{
    if (pos == end())
        throw std::runtime_error("wrong iterator");
    iterator result(pos);
    ++result;
    erase_aux(pos);
    return result;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::size_type
rb_tree<Key, Value, KeyOfValue, Compare>::erase(const key_type &k)
{
    std::pair<iterator, iterator> p = equal_range(k);
    const size_type old_size = size();
    erase(p.first, p.second);
    return old_size - size();
}

// to do
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::base_ptr
rb_tree<Key, Value, KeyOfValue, Compare>::rebalance_for_erase(base_ptr z, base_ptr header)
{
    base_ptr y = z;
    base_ptr x = nullptr, x_parent = nullptr;
    base_ptr &root = header->parent;
    base_ptr &leftmost = header->left;
    base_ptr &rightmost = header->right;
    if (y->left == nullptr)
        x = y->right;
    else if (y->right == nullptr)
        x = y->left;
    else
    {
        y = y->right;
        while (y->left != nullptr)
        {
            y = y->left;
        }
        x = y->right;
    }

    if (y == z)
    {
        x_parent = z->parent;
        if (z == root)
            root = x;
        else if (z == z->parent->left)
            z->parent->left = x;
        else
            z->parent->right = x;
        if (x)
            x->parent = z->parent;
        if (z == leftmost)
        {
            if (z->right == nullptr)
                leftmost = z->parent;
            else
                leftmost = _rb_tree_node_base::minimum(x);
        }
        if (z == rightmost)
        {
            if (z->left == nullptr)
                rightmost = z->parent;
            else
                rightmost = _rb_tree_node_base::maximum(x);
        }
    }
    else
    {
        y->left = z->left;
        z->left->parent = y;
        if (y != z->right)
        {
            x_parent = y->parent;
            if (x)
                x->parent = x_parent;
            y->parent->left = x;
            y->right = z->right;
            z->right->parent = y;
        }
        else
            x_parent = y;
        if (z == root)
            root = y;
        else if (z == z->parent->left)
            z->parent->left = y;
        else
            z->parent->right = y;
        y->parent = z->parent;
        std::swap(y->color, z->color);
        y = z;
    }
    // 如何处理颜色冲突，to do
    return y;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::erase_aux(iterator pos)
{
    link_type del = static_cast<link_type>(rebalance_for_erase(pos.node, this->header));
    destory_node(del);
    --node_count;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::link_type
rb_tree<Key, Value, KeyOfValue, Compare>::M_copy(const rb_tree &rth)
{
    node_count = rth.node_count;
    link_type tmp = M_copy(rth.root(), header);
    leftmost() = static_cast<link_type>(_rb_tree_node_base::minimum(tmp));
    rightmost() = static_cast<link_type>(_rb_tree_node_base::maximum(tmp));
    return tmp;
}

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::link_type
rb_tree<Key, Value, KeyOfValue, Compare>::M_copy(const link_type &node, const base_ptr &p)
{
    if (node == nullptr)
        return nullptr;
    link_type tmp = clone_node(node);
    tmp->parent = p;
    tmp->left = M_copy(left(node), tmp);
    tmp->right = M_copy(right(node), tmp);
    return tmp;
}

//  下面这个erase是第一次自己写的，没有进行后续修正而且没有进行颜色的rebalance
// template <typename Key, typename Value, typename KeyOfValue, typename Compare>
// void rb_tree<Key, Value, KeyOfValue, Compare>::erase_aux(iterator pos)
// {
//     if (left((link_type)(pos.node)) != 0)
//     {
//         link_type y = (link_type)(pos.node);
//         link_type x = left((link_type)(pos.node));
//         while (right(x))
//             y = x, x = right(x);
//         value((link_type)(pos.node)) = x->value;

//         if (x == left(y))
//             left(y) = nullptr;
//         else
//             right(y) = nullptr;
//         if (x == leftmost())
//             leftmost() = (link_type)(pos.node);
//     }
//     else if (right((link_type)(pos.node)) != 0)
//     {
//         link_type y = (link_type)(pos.node);
//         link_type x = right((link_type)(pos.node));
//         while (left(x))
//             y = x, x = left(x);
//         value((link_type)(pos.node)) = x->value;
//         if (x == left(y))
//             left(y) = nullptr;
//         else
//             right(y) = nullptr;
//         if (x == rightmost())
//             rightmost() = (link_type)(pos.node);
//     }
//     else
//     {
//         link_type x = (link_type)(pos.node);
//         link_type y = parent((link_type)(pos.node));
//         if (x == left(y))
//             left(y) = nullptr;
//         if (x == right(y))
//             right(y) = nullptr;
//         if (x == leftmost())
//             leftmost() = (link_type)((++iterator(x)).node);
//         if (x == rightmost())
//             rightmost() = (link_type)((--iterator(x)).node);
//         if (x == root())
//             root() = nullptr;
//     }
//     destory_node(x);
//     --node_count;
// }

template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void rb_tree<Key, Value, KeyOfValue, Compare>::erase(iterator first, iterator last)
{
    if (first == begin() && last == end())
    {
        clear();
    }
    else
    {
        // 注意erase操作会使得first失效，所以可以first=erase(first) 或者erase(first++)
        for (; first != last;)
        {
            erase(first++);
        }
    }
}

int main()
{
    rb_tree<int, int, std::_Identity<int>, std::less<int>> tree;
    tree.insert_unique(2);
    tree.insert_unique(5);
    tree.insert_unique(6);
    rb_tree<int, int, std::_Identity<int>, std::less<int>> t(tree);

    for (auto iter = t.rbegin(); iter != t.rend(); ++iter)
        std::cout << *iter << " ";
    std::cout << std::endl;
}