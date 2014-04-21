#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>

typedef int elem_t;

class Node
{
public:
	elem_t val;
	Node* next;
};

class List
{
public:
	virtual bool insert(elem_t) = 0;
	virtual bool remove(elem_t) = 0;
	virtual bool contains(elem_t) = 0;
	virtual int length() = 0;
	virtual elem_t operator[](int index) = 0;
	Node* _head;
};

#endif