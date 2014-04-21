#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>


template <typename T>
class List
{
public:
	virtual bool insert(T);
	virtual bool remove(T);
	virtual bool contains(T);
	virtual T length();

protected:
	Node* _head;
};

#endif