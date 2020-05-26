#pragma once

#include <cstdint>
#include <iostream>

template <typename T>
struct Node {
    Node(T _value = {}, Node* _left = nullptr, Node* _right = nullptr, Node* _parent = nullptr)
    : value(_value)
    , parent(_parent)
    , left(_left)
    , right(_right) {}

    ~Node() {
        delete left;
        delete right;
        left = right = nullptr;
    }

    T value;

    Node* parent;
    Node* left;
    Node* right;
};

template <typename T>
void Delete(Node<T>* nod) {
    delete nod;
}

template <typename T>
Node<T> *CreateNode(T _value, Node<T> *_left = nullptr, Node<T> *_right = nullptr, Node<T> *_parent = nullptr) {
    Node<T>* result = new Node<T>{_value, _left, _right, _parent};
    if (_left != nullptr) {
        _left->parent = result;
    }
    if (_right != nullptr) {
        _right->parent = result;
    }
    return result;
}

template <typename T>
void Tree_connect(Node<T>* _parent, Node<T>* _left_son = nullptr, Node<T>* _right_son = nullptr) {
    if (_parent == nullptr) {
        return ;
    }
    if (_left_son != nullptr) {
        _parent->left = _left_son;
        _left_son->parent = _parent;
    }
    if (_right_son != nullptr) {
        _parent->right = _right_son;
        _right_son->parent = _parent;
    }
}

template <typename T>
Node<T>* Tree_copy(Node<T>* root) {
    if (root == nullptr) {
        return nullptr;
    }
    Node<T>* new_left = Tree_copy(root->left);
    Node<T>* new_right = Tree_copy(root->right);
    Node<T>* result = CreateNode(root->value, new_left, new_right);

    return result;
}

template <typename T, typename  Func>
void LNR(Node<T>* _current_node, Func _f_) {//std::function
    if (_current_node == nullptr) {
        return ;
    }

    LNR(_current_node->left, _f_);
    _f_(_current_node);
    LNR(_current_node->right, _f_);
}

template <typename T, typename  Func>
void NLR(Node<T>* _current_node, Func _f_) {//std::function
    if (_current_node == nullptr) {
        return ;
    }

    _f_(_current_node);
    NLR(_current_node->left, _f_);
    NLR(_current_node->right, _f_);
}