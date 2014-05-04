#ifndef COARSE_GRAINED_LIST
#define COARSE_GRAINED_LIST

#include <iostream>
#include <pthread.h>

#include "list.h"

template<typename T>
class CoarseGrainedList : public List<T>
{
private:
	pthread_mutex_t* _lock;
	Node<T>* _head;
	int _length;

public:
	CoarseGrainedList();
	virtual bool insert(T val);
	virtual bool remove(T val);
	virtual bool contains(T val);
	virtual int length();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
	T operator[](int index);
	~CoarseGrainedList();
};

#endif
