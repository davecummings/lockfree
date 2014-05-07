#include <stdexcept>
#include <iostream>

#include "fine_grained_list.h"

template<typename K, typename T>
FineGrainedNode<K,T>::FineGrainedNode(K k, T v) : Node<K,T>(k, v)
{
	lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lock, NULL);
	next = NULL;
}

template<typename K, typename T>
FineGrainedNode<K,T>::~FineGrainedNode()
{
	next = NULL;
	pthread_mutex_destroy(lock);
	free(lock);
}

template<typename K, typename T>
FineGrainedList<K,T>::FineGrainedList() : List<K,T>()
{
	_head = NULL;
	_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

template<typename K, typename T>
FineGrainedList<K,T>::~FineGrainedList()
{
	pthread_mutex_lock(_lock);
	FineGrainedNode<K,T>* curr = _head;
	_head = NULL; // break the list
	if (curr != NULL)
		pthread_mutex_lock(curr->lock);

	pthread_mutex_unlock(_lock);

	while (curr != NULL) {
		FineGrainedNode<K,T>* old = curr;
		curr = curr->next;
		if (curr != NULL)
			pthread_mutex_lock(curr->lock);
		pthread_mutex_unlock(old->lock);
		delete old;
	}

	pthread_mutex_destroy(_lock);
	free(_lock);
}

template<typename K, typename T>
void FineGrainedList<K,T>::insert(K key, T val)
{
	pthread_mutex_lock(_lock);

	if (_head == NULL || key < _head->key)
	{
		FineGrainedNode<K,T>* node = new FineGrainedNode<K,T>(key, val);
		node->next = _head;
		_head = node;
		pthread_mutex_unlock(_lock);
		return;
	}

	pthread_mutex_lock(_head->lock);
	FineGrainedNode<K,T>* curr = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<K,T>* next = curr->next;

	if (next == NULL)
	{
		FineGrainedNode<K,T>* node = new FineGrainedNode<K,T>(key, val);
		node->next = NULL;
		curr->next = node;
		pthread_mutex_unlock(curr->lock);
		return;
	}
	else
		pthread_mutex_lock(next->lock);

	while (key > next->key)
	{
		pthread_mutex_unlock(curr->lock);
		curr = next;
		next = next->next;

		if (next == NULL) {
			FineGrainedNode<K,T>* node = new FineGrainedNode<K,T>(key, val);
			node->next = NULL;
			curr->next = node;
			pthread_mutex_unlock(curr->lock);
			return;
		}
		else
			pthread_mutex_lock(next->lock);
	}

	if (key == next->key) {
		next->val = val;
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return;
	}
	else { // val < next->val
		FineGrainedNode<K,T>* node = new FineGrainedNode<K,T>(key, val);
		node->next = next;
		curr->next = node;
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return;
	}
}

template<typename K, typename T>
bool FineGrainedList<K,T>::remove(K key)
{
	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		pthread_mutex_unlock(_lock);
		return false;
	}

	pthread_mutex_lock(_head->lock);

	if (key == _head->key) {
		FineGrainedNode<K,T>* old = _head;
		_head = old->next;
		pthread_mutex_unlock(_lock);
		pthread_mutex_unlock(old->lock);
		delete old;
		return true;
	}

	FineGrainedNode<K,T>* prev = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<K,T>* curr = prev->next;
	if (curr != NULL)
		pthread_mutex_lock(curr->lock);

	while (key > curr->key) {
		pthread_mutex_unlock(prev->lock);
		prev = curr;
		curr = prev->next;
		if (curr != NULL)
			pthread_mutex_lock(curr->lock);
		else {
			pthread_mutex_unlock(prev->lock);
			return false;
		}
	}

	if (key == curr->key) {
		prev->next = curr->next;
		pthread_mutex_unlock(prev->lock);
		pthread_mutex_unlock(curr->lock);
		delete curr;
		return true;
	} else { // val < curr->val
		pthread_mutex_unlock(prev->lock);
		if (curr != NULL)
			pthread_mutex_unlock(curr->lock);

		return false;
	}

}

template<typename K, typename T>
bool FineGrainedList<K,T>::contains(K key)
{
	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		pthread_mutex_unlock(_lock);
		return false;
	} else if (key == _head->key) {
		pthread_mutex_unlock(_lock);
		return true;
	}

	pthread_mutex_lock(_head->lock);
	FineGrainedNode<K,T>* curr = _head; // locked
	pthread_mutex_unlock(_lock);
	FineGrainedNode<K,T>* next = curr->next;

	while (key > curr->key) {
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

	if (key == curr->key) {
		pthread_mutex_unlock(curr->lock);
		return true;
	} else {
		pthread_mutex_unlock(curr->lock);
		pthread_mutex_unlock(next->lock);
		return false;
	}
}

template<typename K, typename T>
T FineGrainedList<K,T>::operator[](K key)
{
	pthread_mutex_lock(_lock);
	FineGrainedNode<K,T>* node = _head;

	while (node != NULL) {
		if (node->key == key) {
			T val = node->val;
			pthread_mutex_unlock(_lock);
			return val;
		}
		node = node->next;
	}

	pthread_mutex_unlock(_lock);
	throw std::out_of_range("Index exceeds list length.");
}

template<typename K, typename T>
bool FineGrainedList<K,T>::isEmpty()
{
	return _head == NULL;
}

template<typename K, typename T>
void FineGrainedList<K,T>::clear()
{
	pthread_mutex_lock(_lock);
	while (_head != NULL) {
		pthread_mutex_lock(_head->lock);
		FineGrainedNode<K,T>* old = _head;
		_head = _head->next;
		pthread_mutex_unlock(old->lock);
		delete old;
	}
	pthread_mutex_unlock(_lock);
}

template<typename K, typename T>
std::string FineGrainedList<K,T>::name()
{
	return "FineGrained";
}

template<typename K, typename T>
int FineGrainedList<K,T>::size()
{
	pthread_mutex_lock(_lock);
	int length = 0;

	while (_head != NULL) {
		pthread_mutex_lock(_head->lock);
		FineGrainedNode<K,T>* old = _head;
		_head = _head->next;
		pthread_mutex_unlock(old->lock);
		length++;
	}

	pthread_mutex_unlock(_lock);
	return length;
}

template class FineGrainedList<int,int>;
template class FineGrainedList<double,int>;
template class FineGrainedList<long,std::string>;
