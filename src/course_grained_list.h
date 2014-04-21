#ifndef COURSE_LIST
#define COURSE_LIST

#include <iostream>
#include <pthread.h>

#include "list.h"

class CourseGrainedList : public List
{

private:
	pthread_mutex_t* _lock;

public:
	CourseGrainedList();
	~CourseGrainedList();
	virtual bool insert(elem_t);
	virtual bool remove(elem_t);
	virtual bool contains(elem_t);
	virtual int length();

};

class CourseGrainedNode : public Node
{

};

#endif