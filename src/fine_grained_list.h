#ifndef FINE_GRAINED_LIST
#define FINE_GRAINED_LIST

#include <pthread.h>

#include "list.h"

template<typename K, typename T>
class FineGrainedNode : public Node<K,T>
{
public:
	FineGrainedNode(K key, T val);
	FineGrainedNode<K,T>* next;
	pthread_mutex_t* lock;
	~FineGrainedNode();
};

template<typename K, typename T>
class FineGrainedList : public List<K,T>
{
private:
	FineGrainedNode<K,T>* _head;
	pthread_mutex_t* _lock;

public:
	FineGrainedList();
	virtual bool insert(K key, T val);
	virtual bool remove(K key);
	virtual bool contains(K key);
	T operator[](K key);
	virtual int size();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
	~FineGrainedList();
};

#endif
