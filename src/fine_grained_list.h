#ifndef FINE_GRAINED_LIST
#define FINE_GRAINED_LIST

#include <pthread.h>

#include "list.h"

template<typename T>
class FineGrainedNode : public Node<T>
{
public:
	FineGrainedNode();
	FineGrainedNode<T>* next;
	pthread_mutex_t* lock;
	~FineGrainedNode();
};

template<typename T>
class FineGrainedList : public List<T>
{
private:
	FineGrainedNode<T>* _head;
	pthread_mutex_t* _lock;
    
public:
	FineGrainedList();
	virtual bool insert(T val);
	virtual bool remove(T val);
	virtual bool contains(T val);
	virtual int length();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
    virtual void printList();
	T operator[](int index);
	~FineGrainedList();
};

#endif
