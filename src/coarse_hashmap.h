#ifndef HASHMAP
#define HASHMAP

#include <iostream>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <functional>
#include <algorithm>

#include "list.h"
#include "coarse_grained_list.h"

template<typename K, typename T, typename H>
class CoarseHashMap
{
private:
    std::vector< CoarseGrainedList<K, T>* >* array;
    std::atomic<int> _size;

public:
    CoarseHashMap()
    {
        array = new std::vector< CoarseGrainedList<K, T>* >;
        _size = 10;
    }

    virtual bool insert(K key, T val);
    virtual bool remove(K key);
    virtual bool contains(K key);
    virtual T operator[](K key);
    virtual void clear();
    virtual int size();
    virtual bool isEmpty();
    virtual std::string name();
    ~CoarseHashMap();
};

#endif
