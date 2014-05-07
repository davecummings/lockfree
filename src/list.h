#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>

template<typename K, typename T>
class Node
{
public:
	K key;
	T val;
	Node<K,T>* next;
	Node(K k, T v) {key = k; val = v;}
};

template<typename K, typename T>
class List
{
public:
	virtual void insert(K key, T val) = 0;
	virtual bool remove(K Key) = 0;
	virtual bool contains(K key) = 0;
	virtual T operator[](K key) = 0;
	virtual int size() = 0;
	virtual bool isEmpty() = 0;
	virtual void clear() = 0;
	virtual std::string name() = 0;

protected:
	Node<K,T>* _head;
};

#endif
