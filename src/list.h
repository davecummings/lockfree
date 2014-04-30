#ifndef LIST
#define LIST

#include <iostream>
#include <cstdlib>


class Node
{
    int elem;
    Node* n;
};

class List
{
public:
	virtual bool insert(int);
	virtual bool delete(int);
	virtual bool find(int);
	virtual int size();
	virtual T operator[](int index) = 0;

protected:
	Node* _head;
};

#endif