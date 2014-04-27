#ifndef COARSE_LIST
#define COARSE_LIST

#include <iostream>
#include <pthread.h>

#include "list.h"

template <typename T>
class CoarseGrainedNode : public Node<T>
{
public:
	CoarseGrainedNode* next;
};

template <typename T>
class CoarseGrainedList : public List<T>
{
private:
	pthread_mutex_t* _lock;

public:
	CoarseGrainedList();
	virtual bool insert(T val);
	virtual bool remove(T val);
	virtual bool contains(T val);
	virtual int length();
	virtual bool isEmpty();
	T operator[](int index);
	~CoarseGrainedList();

private:
	CoarseGrainedNode<T>* _head;
	int _length;
};

#endif