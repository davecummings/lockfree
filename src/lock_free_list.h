#ifndef LOCK_FREE_LIST
#define LOCK_FREE_LIST

#include "list.h"

template<typename K, typename T>
class LockFreeList : public List<K,T>
{
private:
	virtual Node<K,T>* getMarkedReference(Node<K,T>* node);
	virtual Node<K,T>* getUnmarkedReference(Node<K,T>* node);
	virtual bool isMarkedReference(Node<K,T>* node);
	virtual void search(K key,
		Node<K,T>** leftNodeRef, Node<K,T>** rightNodeRef);
	Node<K,T>* _head;
	Node<K,T>* _tail;

public:
	LockFreeList();
	virtual bool insert(K key, T val);
	virtual bool remove(K key);
	virtual bool contains(K key);
	T operator[](K key);
	virtual int size();
	virtual bool isEmpty();
	virtual void clear();
	virtual std::string name();
	virtual void printList();
	~LockFreeList();
};

#endif