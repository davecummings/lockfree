#ifndef COARSE_GRAINED_LIST
#define COARSE_GRAINED_LIST

#include <iostream>
#include <pthread.h>

#include "list.h"

template<typename K, typename T>
class CoarseGrainedList : public List<K,T>
{
private:
	pthread_mutex_t* _lock;
	Node<K,T>* _head;
	int _length;

public:
	CoarseGrainedList();
	virtual void insert(K key, T val);
	virtual bool remove(K key);
	virtual bool contains(K key);
	virtual T operator[](K key);
	virtual int size();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
	~CoarseGrainedList();
};

#endif
