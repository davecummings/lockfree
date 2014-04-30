#include <stdexcept>
#include <iostream>

#include "fine_grained_list.h"

template<typename T>
FineGrainedNode<T>::FineGrainedNode() : Node<T>()
{
	lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lock, NULL);
}

template<typename T>
FineGrainedNode<T>::~FineGrainedNode()
{
	pthread_mutex_destroy(lock);
	// free(lock);
}

template<typename T>
FineGrainedList<T>::FineGrainedList() : List<T>()
{
	_head = NULL;
	_lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

template<typename T>
FineGrainedList<T>::~FineGrainedList()
{
	pthread_mutex_lock(_lock);
	FineGrainedNode<T>* cur = _head;
	_head = NULL; // break the list
	if (cur != NULL) {
		pthread_mutex_lock(cur->lock);
	}
	pthread_mutex_unlock(_lock);

	while (cur != NULL) {
		FineGrainedNode<T>* tmp = cur;
		cur = cur->next;
		if (cur != NULL) {
			pthread_mutex_lock(cur->lock);
		}
		pthread_mutex_unlock(tmp->lock);
		delete tmp;
	}

	pthread_mutex_destroy(_lock);
	free(_lock);
}

template<typename T>
bool FineGrainedList<T>::insert(T val)
{
	FineGrainedNode<T>* n = new FineGrainedNode<T>();
	n->val = val;
	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		n->next = NULL;
		_head = n;
		pthread_mutex_unlock(_lock);
	} else if (val <= _head->val) {
		n->next = _head;
		_head = n;
		pthread_mutex_unlock(_lock);
	} else {
		FineGrainedNode<T>* prev = _head;
		pthread_mutex_lock(prev->lock);
		FineGrainedNode<T>* cur = prev->next;
		if (cur != NULL) {
			pthread_mutex_lock(cur->lock);
		}
		pthread_mutex_unlock(_lock);

		while (cur != NULL) {
			if (val <= cur->val) {
				break;
			}
			if (cur->next != NULL) {
				pthread_mutex_lock(cur->next->lock);
			}
			FineGrainedNode<T>* tmp = prev;
			prev = cur;
			cur = cur->next;
			pthread_mutex_unlock(tmp->lock);
		}

		prev->next = n;
		n->next = cur;

		pthread_mutex_unlock(prev->lock);
		if (cur != NULL) {
			pthread_mutex_unlock(cur->lock);
		}
	}

	return true;
}

template<typename T>
bool FineGrainedList<T>::remove(T val)
{
	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		pthread_mutex_unlock(_lock);
		return false;
	}

	if (_head->val == val) {
		FineGrainedNode<T>* tmp = _head->next;
		delete _head;
		_head = tmp;
		pthread_mutex_unlock(_lock);
		return true;
	}

	FineGrainedNode<T>* prev = _head;
	FineGrainedNode<T>* cur = prev->next;

	pthread_mutex_lock(prev->lock);
	if (cur != NULL) {
		pthread_mutex_lock(cur->lock);
	}

	pthread_mutex_unlock(_lock);

	while (cur != NULL) {
		if (val == cur->val) {
			prev->next = cur->next;
			pthread_mutex_unlock(prev->lock);
			pthread_mutex_unlock(cur->lock);
			delete cur;
			return true;
		}

		FineGrainedNode<T>* tmp = prev;
		prev = cur;
		cur = cur->next;
		if (cur != NULL) {
			pthread_mutex_lock(cur->lock);
		}
		pthread_mutex_unlock(tmp->lock);
	}

	pthread_mutex_unlock(prev->lock);
	return false;
}

template<typename T>
bool FineGrainedList<T>::contains(T val)
{
	pthread_mutex_lock(_lock);

	FineGrainedNode<T>* cur = _head;
	if (cur != NULL) {
		pthread_mutex_lock(cur->lock);
	}
	pthread_mutex_unlock(_lock);

	while (cur != NULL) {
		if (val == cur->val) {
			pthread_mutex_unlock(cur->lock);
			return true;
		}

		FineGrainedNode<T>* tmp = cur;
		cur = cur->next;
		pthread_mutex_unlock(cur->lock);
	}

	return false;
}

template<typename T>
bool FineGrainedList<T>::isEmpty()
{
	return _head == NULL;
}

template<typename T>
int FineGrainedList<T>::length()
{
	pthread_mutex_lock(_lock);

	FineGrainedNode<T>* node = _head;
	int length = 0;

	while (node != NULL) {
		node = node->next;
		length++;
	}
	pthread_mutex_unlock(_lock);
	return length;
}

template<typename T>
T FineGrainedList<T>::operator[](int index)
{
	pthread_mutex_lock(_lock);
	int i = 0;
	FineGrainedNode<T>* node = _head;

	while (node != NULL) {
		if (index == i) {
			pthread_mutex_unlock(_lock);
			return node->val;
		}
		i++;
		node = node->next;
	}

	pthread_mutex_unlock(_lock);
	throw std::out_of_range("Index exceeds list length.");
}

template class FineGrainedList<int>;
template class FineGrainedList<double>;
template class FineGrainedList<long>;
