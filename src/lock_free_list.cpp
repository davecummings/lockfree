#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "lock_free_list.h"

template<typename K, typename T>
LockFreeList<K,T>::LockFreeList() : List<K,T>()
{
	_head = new Node<K,T>(0,0);
	_tail = new Node<K,T>(0,0);
	_head->next = _tail;
}

template<typename K, typename T>
LockFreeList<K,T>::~LockFreeList()
{
	while (_head->next != _tail)
		remove(_head->next->key);

	delete _head;
	delete _tail;
}

template<typename K, typename T>
Node<K,T>* LockFreeList<K,T>::getMarkedReference(Node<K,T>* n)
{
	long p = (long) n;
	p = p | 1;
	return (Node<K,T>*) p;
}

template<typename K, typename T>
Node<K,T>* LockFreeList<K,T>::getUnmarkedReference(Node<K,T>* n)
{
	long p = (long) n;
	p = (p >> 1) << 1;
	return (Node<K,T>*) p;
}

template<typename K, typename T>
bool LockFreeList<K,T>::isMarkedReference(Node<K,T>* n)
{
	long p = (long) n;
	p = p & 1;
	return (bool) p;
}

template<typename K, typename T>
void LockFreeList<K,T>::search(K key,
	Node<K,T>** leftNodeRef, Node<K,T>** rightNodeRef)
{
	Node<K,T>* leftNodeNext = NULL;

	while (true)
	{
		Node<K,T>* prev = _head;
		Node<K,T>* cur = prev->next;

		/* 1: Find leftNode and rightNode */
		do
		{
			if (!isMarkedReference(cur))
			{
				*leftNodeRef = prev;
				leftNodeNext = cur;
			}
			prev = getUnmarkedReference(cur);
			if (prev == _tail)
				break;
			cur = prev->next;
		} while (isMarkedReference(cur) || prev->key < key);
		*rightNodeRef = prev;

		/* 2: Check if nodes are adjacent */
		if (leftNodeNext == *rightNodeRef)
		{
			if (*rightNodeRef != _tail &&
				isMarkedReference((*rightNodeRef)->next))
				continue;
			else return;
		}

		/* 3: Remove one or more marked nodes */
		if (__sync_bool_compare_and_swap(&((*leftNodeRef)->next),
			leftNodeNext, *rightNodeRef))
		{
			// right node got marked while performing operation
			if (*rightNodeRef != _tail &&
				isMarkedReference((*rightNodeRef)->next))
				continue;
			// free leftNode through rightNode, exclusive
			else return;
		}
	}
}

template<typename K, typename T>
bool LockFreeList<K,T>::insert(K key, T val)
{
	Node<K,T>* n = new Node<K,T>(key, val);
	Node<K,T>* leftNode;
	Node<K,T>* rightNode;

	while (true) {
		search(key, &leftNode, &rightNode);
		// already in the list
		if (rightNode != _tail && rightNode->key == key)
		{
			rightNode->val = val;
			delete n;
			return false;
		}
		else
		{
			n->next = rightNode;
			if (__sync_bool_compare_and_swap(&leftNode->next, rightNode, n))
				return true;
		}
	}
}

template<typename K, typename T>
bool LockFreeList<K,T>::remove(K key)
{
	Node<K,T>* leftNode;
	Node<K,T>* rightNode;
	Node<K,T>* rightNodeNext;

	while (true) {
		search(key, &leftNode, &rightNode);

		// if node not in list
		if (rightNode == _tail || rightNode->key != key)
			return false;

		rightNodeNext = rightNode->next;

		// mark for deletion if not already
		if (!isMarkedReference(rightNodeNext))
		{
			if (__sync_bool_compare_and_swap(&rightNode->next, rightNodeNext,
					getMarkedReference(rightNodeNext))) break;
		}
	}

	/* Try to physically delete the node here.
		Otherwise, call search to delete it. */
	if (!__sync_bool_compare_and_swap(&leftNode->next, rightNode, rightNodeNext))
		search(rightNode->key, &leftNode, &rightNode);

	return true;
}

template<typename K, typename T>
bool LockFreeList<K,T>::contains(K key)
{
	Node<K,T>* leftNode;
	Node<K,T>* rightNode;

	search(key, &leftNode, &rightNode);
	if (rightNode == _tail || rightNode->key != key)
		return false;
	else
		return true;
}

template<typename K, typename T>
T LockFreeList<K,T>::operator[](K key)
{
	Node<K,T>* leftNode;
	Node<K,T>* rightNode;

	search(key, &leftNode, &rightNode);
	if (rightNode == _tail || rightNode->key != key)
		throw std::out_of_range("Key not in set.");
	else
		return rightNode->val;
}

template<typename K, typename T>
bool LockFreeList<K,T>::isEmpty()
{
	return _head->next == _tail;
}

template<typename K, typename T>
int LockFreeList<K,T>::size()
{
	// Node<T;>* leftNode;
	// Node<T>** leftNodeRef = &leftNode;
	// Node<T>* rightNode;
	// Node<T>** rightNodeRef = &rightNode;
	// Node<T>* leftNodeNext;

	// int length = 0;

	// while (true) {
	// 	Node<T>* prev = _head;
	// 	Node<T>* cur = prev->next;

	// 	/* 1: Find leftNode and rightNode */
	// 	while (true) {
	// 		if (!isMarkedReference(cur)) {
	// 			*leftNodeRef = prev;
	// 			leftNodeNext = cur;
	// 		}
	// 		prev = getUnmarkedReference(cur);
	// 		if (prev == _tail) {
	// 			break;
	// 		}
	// 		cur = prev->next;
	// 		length++;
	// 	}
	// 	*rightNodeRef = prev;

	// 	 2: Check if nodes are adjacent
	// 	if (leftNodeNext == *rightNodeRef) {
	// 		if (*rightNodeRef != _tail && isMarkedReference((*rightNodeRef)->next)) {
	// 			continue;
	// 		} else {
	// 			break;
	// 		}
	// 	}

	// 	/* 3: Remove one or more marked nodes */
	// 	if (__sync_bool_compare_and_swap(&((*leftNodeRef)->next), leftNodeNext, *rightNodeRef)) {
	// 		// right node got marked while performing operation
	// 		if (*rightNodeRef != _tail && isMarkedReference((*rightNodeRef)->next)) {
	// 			continue;
	// 		} else {
	// 			break;
	// 		}
	// 	}
	// }

	// return length;

	Node<K,T>* node = getUnmarkedReference(_head->next);
	int length = 0;

	while (node != _tail && node != NULL) {
		length++;
		node = getUnmarkedReference(node->next);
	}

	return length;
}

template<typename K, typename T>
void LockFreeList<K,T>::clear()
{
	Node<K,T>* node = getUnmarkedReference(_head);
	Node<K,T>* tail = getUnmarkedReference(_tail);
	_head = new Node<K,T>(0,0);
	_tail = new Node<K,T>(0,0);
	_head->next = _tail;

	while (node != tail) {
		Node<K,T>* prev = node;
		node = node->next;
		delete prev;
	}

	delete tail;
}

template<typename K, typename T>
std::string LockFreeList<K,T>::name()
{
	return "LockFree";
}

template<typename K, typename T>
void LockFreeList<K,T>::printList()
{
	std::cout << "[";

	Node<K,T>* cur = getUnmarkedReference(_head);

	while (true) {
		cur = getUnmarkedReference(cur->next);
		if (cur == _tail || cur == NULL) {
			break;
		}
		std::cout << "(" << cur->key << "," << cur->val << "),";
	}

	std::cout << "]" << std::endl;

}

template class LockFreeList<int, int>;
