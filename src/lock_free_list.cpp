#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "lock_free_list.h"

template<typename T>
LockFreeList<T>::LockFreeList() : List<T>()
{
	_head = new Node<T>();
	_tail = new Node<T>();
	_head->next = _tail;
}

template<typename T>
LockFreeList<T>::~LockFreeList()
{
	while (_head->next != _tail) {
		remove(_head->next->val);
	}
	delete _head;
	delete _tail;
}

template<typename T>
Node<T>* LockFreeList<T>::getMarkedReference(Node<T>* n)
{
	long p = (long) n;
	p = p | 1;
	return (Node<T>*) p;
}

template<typename T>
Node<T>* LockFreeList<T>::getUnmarkedReference(Node<T>* n)
{
	long p = (long) n;
	p = (p >> 1) << 1;
	return (Node<T>*) p;
}

template<typename T>
bool LockFreeList<T>::isMarkedReference(Node<T>* n)
{
	long p = (long) n;
	p = p & 1;
	return (bool) p;
}

template<typename T>
void LockFreeList<T>::search(T val, Node<T>** leftNodeRef, Node<T>** rightNodeRef)
{
	Node<T>* leftNodeNext = NULL;

	while (true) {
		Node<T>* prev = _head;
		Node<T>* cur = prev->next;

		/* 1: Find leftNode and rightNode */
		do {
			if (!isMarkedReference(cur)) {
				*leftNodeRef = prev;
				leftNodeNext = cur;
			}
			prev = getUnmarkedReference(cur);
			if (prev == _tail) {
				break;
			}
			cur = prev->next;
		} while (isMarkedReference(cur) || prev->val < val);
		*rightNodeRef = prev;

		/* 2: Check if nodes are adjacent */
		if (leftNodeNext == *rightNodeRef) {
			if (*rightNodeRef != _tail && isMarkedReference((*rightNodeRef)->next)) {
				continue;
			} else {
				return;
			}
		}

		/* 3: Remove one or more marked nodes */
		if (__sync_bool_compare_and_swap(&((*leftNodeRef)->next), leftNodeNext, *rightNodeRef)) {
			// right node got marked while performing operation
			if (*rightNodeRef != _tail && isMarkedReference((*rightNodeRef)->next)) {
				continue;
			} else {
				// free leftNode through rightNode, inclusive
				return;
			}
		}
	}
}

template<typename T>
bool LockFreeList<T>::insert(T val)
{
	Node<T>* n = new Node<T>();
	n->val = val;
	Node<T>* leftNode;
	Node<T>* rightNode;

	while (true) {
		search(val, &leftNode, &rightNode);
		// already in the list
		if (rightNode != _tail && rightNode->val == val) {
			return false;
		}
		n->next = rightNode;
		if (__sync_bool_compare_and_swap(&leftNode->next, rightNode, n)) {
			return true;
		}
	}
}

template<typename T>
bool LockFreeList<T>::remove(T val)
{
	Node<T>* leftNode;
	Node<T>* rightNode;
	Node<T>* rightNodeNext;

	while (true) {
		search(val, &leftNode, &rightNode);

		// if node not in list
		if (rightNode == _tail || rightNode->val != val) {
			return false;
		}
		rightNodeNext = rightNode->next;

		// mark for deletion if not already
		if (!isMarkedReference(rightNodeNext)) {
			if (__sync_bool_compare_and_swap(&rightNode->next, rightNodeNext,
					getMarkedReference(rightNodeNext))) {
				break;
			}
		}
	}

	/* Try to physically delete the node here.
		Otherwise, call search to delete it. */
	if (!__sync_bool_compare_and_swap(&leftNode->next, rightNode, rightNodeNext)) {
		search(rightNode->val, &leftNode, &rightNode);
	} else {
		// free rightNode
	}

	return true;
}

template<typename T>
bool LockFreeList<T>::contains(T val)
{
	Node<T>* leftNode;
	Node<T>* rightNode;

	search(val, &leftNode, &rightNode);
	if (rightNode == _tail || rightNode->val != val) {
		return false;
	} else {
		return true;
	}
}

template<typename T>
bool LockFreeList<T>::isEmpty()
{
	return _head->next == _tail;
}

template<typename T>
int LockFreeList<T>::length()
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

	Node<T>* node = getUnmarkedReference(_head->next);
	int length = 0;

	while (node != _tail && node != NULL) {
		length++;
		node = node->next;
		node = getUnmarkedReference(node);
	}

	return length;
}

template<typename T>
void LockFreeList<T>::clear()
{
	Node<T>* node = getUnmarkedReference(_head);
	Node<T>* tail = getUnmarkedReference(_tail);
	_head = new Node<T>();
	_tail = new Node<T>();
	_head->next = _tail;

	while (node != tail) {
		Node<T>* prev = node;
		node = node->next;
		delete prev;
	}

	delete tail;
}

template<typename T>
std::string LockFreeList<T>::name()
{
	return "LockFree";
}

template<typename T>
void LockFreeList<T>::printList()
{
	std::cout << "[";

	Node<T>* cur = getUnmarkedReference(_head);

	while (true) {
		cur = getUnmarkedReference(cur->next);
		if (cur == _tail || cur == NULL) {
			break;
		}
		std::cout << cur->val << ",";
	}

	std::cout << "]" << std::endl;

}

template<typename T>
T LockFreeList<T>::operator[](int index)
{
	Node<T>* node = getUnmarkedReference(_head->next);
	int i = 0;

	while (node != _tail && node != NULL) {
		if (index == i) {
			return node->val;
		}
		i++;
		node = node->next;
		node = getUnmarkedReference(node);
	}

	std::cout << "Index: " << index << std::endl;
	throw std::out_of_range("Index exeeds list length.");
}

template class LockFreeList<int>;
template class LockFreeList<double>;
template class LockFreeList<long>;
