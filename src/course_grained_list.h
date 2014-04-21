#ifndef COARSE_LIST
#define COARSE_LIST

#include <iostream>
#include <pthread.h>

#include "list.h"


template <typename T>
class CoarseGrainedList : public List<T>
{
private:
	pthread_mutex_t* _lock;

public:
	CourseGrainedList();
	~CourseGrainedList();
	virtual bool insert(T);
	virtual bool remove(T);
	virtual bool contains(T);
	virtual int length();
	T operator[](int index);
};

class CoarseGrainedNode : public Node
{

};

#endif