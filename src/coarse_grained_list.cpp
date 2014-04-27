#include <stdexcept>

#include "coarse_grained_list.h"

template<typename T>
CoarseGrainedList<T>::CoarseGrainedList() : List<T>()
{
	_head = NULL;
    _length = 0;
    _lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

template<typename T>
CoarseGrainedList<T>::~CoarseGrainedList()
{
    pthread_mutex_lock(_lock);

	CoarseGrainedNode<T>* node = _head;
	while (node != NULL)
    {
		CoarseGrainedNode<T>* tmp = node;
		node = node->next;
		delete tmp;
	}

    pthread_mutex_unlock(_lock);
	pthread_mutex_destroy(_lock);
    free(_lock);
}

template<typename T>
bool CoarseGrainedList<T>::insert(T val)
{
	CoarseGrainedNode<T>* node = new CoarseGrainedNode<T>();
	if (node == NULL) {
		return false;
	}
	node->val = val;

	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		node->next = NULL;
		_head = node;
	} else if (val < _head->val) {
		node->next = _head;
		_head = node;

	} else {
		CoarseGrainedNode<T>* cur = _head->next;
		CoarseGrainedNode<T>* prev = _head;

		while (cur != NULL) {
			if (val < cur->val) {
				break;
			}
			prev = cur;
			cur = cur->next;
		}

		prev->next = node;
		node->next = cur;
	}

    _length++;
	pthread_mutex_unlock(_lock);
	return true;
}

template<typename T>
bool CoarseGrainedList<T>::remove(T val)
{
    pthread_mutex_lock(_lock);

    if (_head == NULL) {
        pthread_mutex_unlock(_lock);
        return false;
    }

    if (_head->val == val) {
        CoarseGrainedNode<T>* tmp = _head->next;
        delete _head;
        _head = tmp;
        _length--;
        pthread_mutex_unlock(_lock);
        return true;
    }

    CoarseGrainedNode<T>* prev = _head;
    CoarseGrainedNode<T>* cur = prev->next;
    while (cur != NULL) {
        if (val == cur->val) {
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

template<typename T>
bool CoarseGrainedList<T>::contains(T val)
{
    pthread_mutex_lock(_lock);

    CoarseGrainedNode<T>* cur = _head;
    while (cur != NULL) {
        if (val == cur->val) {
            pthread_mutex_unlock(_lock);
            return true;
        }

        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    return false;
}

template<typename T>
int CoarseGrainedList<T>::length()
{
    return _length;
}

template<typename T>
bool CoarseGrainedList<T>::isEmpty()
{
    return _head == NULL;
}

template<typename T>
T CoarseGrainedList<T>::operator[](int index)
{
    pthread_mutex_lock(_lock);
    int i = 0;
    CoarseGrainedNode<T>* node = _head;

    while (node != NULL) {
        if (index == i) {
            pthread_mutex_unlock(_lock);
            return node->val;
        }
        i++;
        node = node->next;
    }

    throw std::out_of_range("No such element in list.");
}

template class CoarseGrainedList<int>;