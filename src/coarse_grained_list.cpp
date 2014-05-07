#include <stdexcept>
#include <iostream>
#include <pthread.h>

#include "list.h"
#include "coarse_grained_list.h"


template<typename K, typename T>
CoarseGrainedList<K,T>::CoarseGrainedList() : List<K,T>()
{
	_head = NULL;
    _length = 0;
    _lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

template<typename K, typename T>
CoarseGrainedList<K,T>::~CoarseGrainedList()
{
    pthread_mutex_lock(_lock);

	Node<K,T>* node = _head;
	while (node != NULL)
    {
		Node<K,T>* tmp = node;
		node = node->next;
		delete tmp;
	}

    pthread_mutex_unlock(_lock);
	pthread_mutex_destroy(_lock);
    free(_lock);
}

template <typename K, typename T>
void CoarseGrainedList<K,T>::insert(K key, T val)
{
	pthread_mutex_lock(_lock);

	if (_head == NULL)
    {
        Node<K,T>* node = new Node<K,T>(key, val);
		node->next = NULL;
		_head = node;
        _length++;
        pthread_mutex_unlock(_lock);
        return;
    }

	if (key < _head->key)
    {
        Node<K,T>* node = new Node<K,T>(key, val);
		node->next = _head;
		_head = node;
        _length++;
        pthread_mutex_unlock(_lock);
        return;
	}

    if (key == _head->key)
    {
        _head->val = val;
        pthread_mutex_unlock(_lock);
        return;
    }

	Node<K,T>* cur = _head->next;
	Node<K,T>* prev = _head;

	while (cur != NULL) {
		if (key < cur->key)
            break;
		if (key == cur->key)
        {
            cur->val = val;
            pthread_mutex_unlock(_lock);
            return;
        }
		prev = cur;
		cur = cur->next;
	}

    Node<K,T>* node = new Node<K,T>(key, val);
    prev->next = node;
    node->next = cur;
    _length++;
    pthread_mutex_unlock(_lock);
}

template<typename K, typename T>
bool CoarseGrainedList<K,T>::remove(K key)
{
    pthread_mutex_lock(_lock);

    if (_head == NULL)
    {
        pthread_mutex_unlock(_lock);
        return false;
    }

    if (_head->key == key)
    {
        Node<K,T>* tmp = _head->next;
        delete _head;
        _head = tmp;
        _length--;
        pthread_mutex_unlock(_lock);
        return true;
    }

    Node<K,T>* prev = _head;
    Node<K,T>* cur = prev->next;

    while (cur != NULL)
    {
        if (key == cur->key)
        {
            prev->next = cur->next;
            delete cur;
            _length--;
            pthread_mutex_unlock(_lock);
            return true;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    return false;
}

template<typename K, typename T>
bool CoarseGrainedList<K,T>::contains(K key)
{
    pthread_mutex_lock(_lock);

    Node<K,T>* cur = _head;
    while (cur != NULL) {
        if (key == cur->key) {
            pthread_mutex_unlock(_lock);
            return true;
        }

        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    return false;
}

template<typename K, typename T>
T CoarseGrainedList<K,T>::operator[](K key)
{
    pthread_mutex_lock(_lock);

    Node<K,T>* cur = _head;
    while (cur != NULL) {
        if (key == cur->key) {
            T val = cur->val;
            pthread_mutex_unlock(_lock);
            return val;
        }

        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    throw std::out_of_range("Key not in set");
}

template<typename K, typename T>
int CoarseGrainedList<K,T>::size()
{
    return _length;
}

template<typename K, typename T>
bool CoarseGrainedList<K,T>::isEmpty()
{
    return _length == 0;
}

template<typename K, typename T>
void CoarseGrainedList<K,T>::clear()
{
    pthread_mutex_lock(_lock);
    Node<K,T>* node = _head;
    _head = NULL;
    _length = 0;

    while (node != NULL) {
        Node<K,T>* prev = node;
        node = node->next;
        delete prev;
    }

    pthread_mutex_unlock(_lock);
}

template<typename K, typename T>
std::string CoarseGrainedList<K,T>::name()
{
    return "CoarseGrained";
}

template class CoarseGrainedList<int, int>;
template class CoarseGrainedList<double, int>;
template class CoarseGrainedList<long, float>;
