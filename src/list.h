#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>


class Node
{
    int elemt;
    Node* n;
};

template <typename T>
class List
{
public:
	virtual bool insert(T) = 0;
	virtual bool remove(T) = 0;
	virtual bool contains(T) = 0;
	virtual int length() = 0;
	virtual T operator[](int index) = 0;

protected:
	Node* _head;
};

#endif