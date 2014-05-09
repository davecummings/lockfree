#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "coarse_hashmap.h"

using namespace std;


// const static double LOAD_FACTOR = .5;

template<typename K, typename T, typename H>
bool CoarseHashMap<K,T,H>::insert(K key, T val)
{
    int hash = H(key);
    CoarseGrainedList<K, T> list = array[hash];
    if (list->insert(key, val))
    {
        _size++;
        return true;
    }
    else
        return false;
}

template<typename K, typename T, typename H>
bool CoarseHashMap<K,T,H>::remove(K key)
{
    int hash = H(key);
    CoarseGrainedList<K, T> list = array[hash];
    if (list->remove(key))
    {
        _size--;
        return true;
    }
    else
        return false;
}

template<typename K, typename T, typename H>
bool CoarseHashMap<K,T,H>::contains(K key)
{
    int hash = H(key);
    CoarseGrainedList<K, T> list = array[hash];
    return list->contains(key);
}

template<typename K, typename T, typename H>
T CoarseHashMap<K,T,H>::operator[](K key)
{
    int hash = H(key);
    CoarseGrainedList<K, T> list = array[hash];
    return list[key];
}

template<typename K, typename T, typename H>
void CoarseHashMap<K,T,H>::clear()
{
    for (typename std::vector< CoarseGrainedList<K, T>* >::iterator it = array->begin(); it != array->end(); ++it)
        it.clear();
}

template<typename K, typename T, typename H>
int CoarseHashMap<K,T,H>::size()
{
    return _size;
}

template<typename K, typename T, typename H>
bool CoarseHashMap<K,T,H>::isEmpty()
{
    return _size == 0;
}

template<typename K, typename T, typename H>
std::string CoarseHashMap<K,T,H>::name()
{
    return "HashMap using " + array[0].name();
}
