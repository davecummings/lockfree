#ifndef LOCK_FREE_LIST
#define LOCK_FREE_LIST

#include "list.h"

template<typename T>
class LockFreeList : public List<T>
{
private:
	virtual Node<T>* getMarkedReference(Node<T>* node);
	virtual Node<T>* getUnmarkedReference(Node<T>* node);
	virtual bool isMarkedReference(Node<T>* node);
	virtual void search(T val, Node<T>** leftNodeRef, Node<T>** rightNodeRef);
	Node<T>* _head;
	Node<T>* _tail;

public:
	LockFreeList();
	virtual bool insert(T val);
	virtual bool remove(T val);
	virtual bool contains(T val);
	virtual int length();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
	virtual void printList();
	T operator[](int index);
	~LockFreeList();
};

#endif