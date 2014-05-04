#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>


class Node
{
public:
	T val;
	Node<T>* next;
};

class List
{
public:
	virtual bool insert(T val) = 0;
	virtual bool remove(T val) = 0;
	virtual bool contains(T val) = 0;
	virtual int length() = 0;
	virtual bool isEmpty() = 0;
	virtual void clear() = 0;
	virtual std::string name() = 0;
	virtual T operator[](int index) = 0;

protected:
	Node<T>* _head;
};

#endif
