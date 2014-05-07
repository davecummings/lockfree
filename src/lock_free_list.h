#ifndef LOCK_FREE_LIST
#define LOCK_FREE_LIST

#include "list.h"

template<typename T>
class LockFreeList : public List<T>
{
private:
	virtual void search(T val, Node<T>** leftNodeRef, Node<T>** rightNodeRef);
	Node<T>* _head;
	Node<T>* _tail;
    virtual Node<T>* getMarkedReference(Node<T>* n);
    virtual Node<T>* getUnmarkedReference(Node<T>* n);
    virtual bool isMarkedReference(Node<T>* n);
    
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