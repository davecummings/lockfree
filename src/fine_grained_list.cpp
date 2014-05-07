#include <stdexcept>
#include <iostream>

#include "fine_grained_list.h"

template<typename T>
FineGrainedNode<T>::FineGrainedNode() : Node<T>()
{
	lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lock, NULL);
	next = NULL;
}

template<typename T>
FineGrainedNode<T>::~FineGrainedNode()
{
	next = NULL;
	pthread_mutex_destroy(lock);
	free(lock);
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
	FineGrainedNode<T>* curr = _head;
	_head = NULL; // break the list
	if (curr != NULL) {
		pthread_mutex_lock(curr->lock);
	}
	pthread_mutex_unlock(_lock);
    
	while (curr != NULL) {
		FineGrainedNode<T>* old = curr;
		curr = curr->next;
		if (curr != NULL) {
			pthread_mutex_lock(curr->lock);
		}
		pthread_mutex_unlock(old->lock);
		delete old;
	}
    
	pthread_mutex_destroy(_lock);
	free(_lock);
}

template<typename T>
bool FineGrainedList<T>::insert(T val)
{
	pthread_mutex_lock(_lock);
    
	if (_head == NULL || val < _head->val) {
		FineGrainedNode<T>* node = new FineGrainedNode<T>();
		node->val = val;
		node->next = _head;
		_head = node;
		pthread_mutex_unlock(_lock);
		return true;
	}
    
	pthread_mutex_lock(_head->lock);
	FineGrainedNode<T>* curr = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<T>* next = curr->next;
    
	if (next == NULL) {
		FineGrainedNode<T>* node = new FineGrainedNode<T>();
		node->val = val;
		node->next = NULL;
		curr->next = node;
		pthread_mutex_unlock(curr->lock);
		return true;
	} else {
		pthread_mutex_lock(next->lock);
	}
    
	while (val > next->val) {
		pthread_mutex_unlock(curr->lock);
		curr = next;
		next = next->next;
        
		if (next == NULL) {
			FineGrainedNode<T>* node = new FineGrainedNode<T>();
			node->val = val;
			node->next = NULL;
			curr->next = node;
			pthread_mutex_unlock(curr->lock);
			return true;
		} else {
			pthread_mutex_lock(next->lock);
		}
	}
    
	if (val == next->val) {
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return false;
	} else { // val < next->val
		FineGrainedNode<T>* node = new FineGrainedNode<T>();
		node->val = val;
		node->next = next;
		curr->next = node;
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return true;
	}
}

template<typename T>
bool FineGrainedList<T>::remove(T val)
{
	pthread_mutex_lock(_lock);
    
	if (_head == NULL) {
		pthread_mutex_unlock(_lock);
		return false;
	}
    
	pthread_mutex_lock(_head->lock);
    
	if (val == _head->val) {
		FineGrainedNode<T>* old = _head;
		_head = old->next;
		pthread_mutex_unlock(_lock);
		pthread_mutex_unlock(old->lock);
		delete old;
		return true;
	}
    
	FineGrainedNode<T>* prev = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<T>* curr = prev->next;
	if (curr != NULL) {
		pthread_mutex_lock(curr->lock);
	}
    
	while (val > curr->val) {
		pthread_mutex_unlock(prev->lock);
		prev = curr;
		curr = prev->next;
		if (curr != NULL) {
			pthread_mutex_lock(curr->lock);
		} else {
			pthread_mutex_unlock(prev->lock);
			return false;
		}
	}
    
	if (val == curr->val) {
		prev->next = curr->next;
		pthread_mutex_unlock(prev->lock);
		pthread_mutex_unlock(curr->lock);
		delete curr;
		return true;
	} else { // val < curr->val
		pthread_mutex_unlock(prev->lock);
		if (curr != NULL) {
			pthread_mutex_unlock(curr->lock);
		}
		return false;
	}
    
}

template<typename T>
bool FineGrainedList<T>::contains(T val)
{
	pthread_mutex_lock(_lock);
    
	if (_head == NULL) {
		pthread_mutex_unlock(_lock);
		return false;
	} else if (val == _head->val) {
		pthread_mutex_unlock(_lock);
		return true;
	}
    
	pthread_mutex_lock(_head->lock);
	FineGrainedNode<T>* curr = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<T>* next = curr->next;
    
	while (val > curr->val) {
		if (next == NULL) {
			pthread_mutex_unlock(curr->lock);
			return false;
		} else {
			pthread_mutex_lock(next->lock);
			pthread_mutex_unlock(curr->lock);
			curr = next;
			next = next->next;
		}
	}
    
	if (val == curr->val) {
		pthread_mutex_unlock(curr->lock);
		return true;
	} else {
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return false;
	}
}

template<typename T>
bool FineGrainedList<T>::isEmpty()
{
	return _head == NULL;
}

template<typename T>
void FineGrainedList<T>::clear()
{
	pthread_mutex_lock(_lock);
	while (_head != NULL) {
		pthread_mutex_lock(_head->lock);
		FineGrainedNode<T>* old = _head;
		_head = _head->next;
		pthread_mutex_unlock(old->lock);
		delete old;
	}
	pthread_mutex_unlock(_lock);
}

template<typename T>
std::string FineGrainedList<T>::name()
{
	return "FineGrained";
}

template<typename T>
int FineGrainedList<T>::length()
{
	pthread_mutex_lock(_lock);
	int length = 0;
    
    FineGrainedNode<T>* node = _head;
	while (node != NULL) {
		pthread_mutex_lock(node->lock);
		pthread_mutex_unlock(node->lock);
		node = node->next;
		length++;
	}
    
	pthread_mutex_unlock(_lock);
	return length;
}

template<typename T>
void FineGrainedList<T>::printList()
{
	pthread_mutex_lock(_lock);
    std::cout << "[";
    
    FineGrainedNode<T>* node = _head;
	while (node != NULL) {
		pthread_mutex_lock(node->lock);
		pthread_mutex_unlock(node->lock);;
        std::cout << node->val << ",";
        node = node->next;
	}
    std::cout << "]" << std::endl;
    
	pthread_mutex_unlock(_lock);
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
