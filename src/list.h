#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>

template <typename T>
class Node
{
public:
	T val;
	Node* next;
};

template <typename T>
class List
{
public:
	virtual bool insert(T val) = 0;
	virtual bool remove(T val) = 0;
	virtual bool contains(T val) = 0;
	virtual int length() = 0;
	virtual bool isEmpty() = 0;
	virtual T operator[](int index) = 0;
};

#endif
