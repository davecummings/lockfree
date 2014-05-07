#ifndef HASHMAP
#define HASHMAP

#include <iostream>
#include <cstdlib>
#include <vector>

#include "list.h"


template<typename K, typename T,  typename L = List<K,T>, typename H = std::hash<K> >
class HashMap : public List<K,T>
{
public:
    HashMap(int size);
    void insert(K key, T val);
    bool remove(K key);
    bool contains(K key);
    T operator[](K key);
    void clear();
    std::string name() = 0;
    ~HashMap();

private:
    std::vector<L> array;
};

#endif
