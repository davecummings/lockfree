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
};

class CoarseGrainedNode : public Node
{

};

#endif