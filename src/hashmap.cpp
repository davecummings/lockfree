#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "hashmap.h"

using namespace std;


template<typename K, typename T, typename L, typename H>
HashMap<K,T,L,H>::HashMap(int size)
{
    array = new std::vector<L>(size);
}

template<typename K, typename T, typename L, typename H>
HashMap<K,T,L,H>::~HashMap()
{
    for (typename std::vector<L>::iterator it = array.begin(); it != array.end(); ++it)
        delete *it;
}

template<typename K, typename T, typename L, typename H>
void HashMap<K,T,L,H>::insert(K key, T val)
{
    int hash = H(key);
    L list = array[hash];
    list->insert(key, val);
}

template<typename K, typename T, typename L, typename H>
bool HashMap<K,T,L,H>::remove(K key)
{
    int hash = H(key);
    L list = array[hash];
    return list->remove(key);
}

template<typename K, typename T, typename L, typename H>
bool HashMap<K,T,L,H>::contains(K key)
{
    int hash = H(key);
    L list = array[hash];
    return list->contains(key);
}

template<typename K, typename T, typename L, typename H>
T HashMap<K,T,L,H>::operator[](K key)
{
    int hash = H(key);
    L list = array[hash];
    return list[key];
}

template<typename K, typename T, typename L, typename H>
void HashMap<K,T,L,H>::clear()
{
    for (typename std::vector<L>::iterator it = array.begin(); it != array.end(); ++it)
        it.clear();
}

template<typename K, typename T, typename L, typename H>
std::string HashMap<K,T,L,H>::name()
{
    return "HashMap";
}
