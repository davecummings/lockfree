#include <stdexcept>

#include "coarse_grained_list.h"

template<typename T>
CoarseGrainedList<T>::CoarseGrainedList() : List<T>()
{
	_head = NULL;
    _length = 0;
    _lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

template<typename T>
CoarseGrainedList<T>::~CoarseGrainedList()
{
    pthread_mutex_lock(_lock);
    
	Node<T>* node = _head;
	while (node != NULL)
    {
		Node<T>* tmp = node;
		node = node->next;
		delete tmp;
	}
    
    pthread_mutex_unlock(_lock);
	pthread_mutex_destroy(_lock);
    free(_lock);
}

template<typename T>
bool CoarseGrainedList<T>::insert(T val)
{
	Node<T>* node = new Node<T>();
    
	node->val = val;
    
	pthread_mutex_lock(_lock);
    
	if (_head == NULL) {
		node->next = NULL;
		_head = node;
	} else if (val < _head->val) {
		node->next = _head;
		_head = node;
        
	} else if (val == _head->val) {
        pthread_mutex_unlock(_lock);
        return false;
    } else {
		Node<T>* cur = _head->next;
		Node<T>* prev = _head;
        
		while (cur != NULL) {
			if (val < cur->val) {
				break;
			} else if (val == cur->val) {
                pthread_mutex_unlock(_lock);
                return false;
            }
			prev = cur;
			cur = cur->next;
		}
        
		prev->next = node;
		node->next = cur;
	}
    
    _length++;
	pthread_mutex_unlock(_lock);
	return true;
}

template<typename T>
bool CoarseGrainedList<T>::remove(T val)
{
    pthread_mutex_lock(_lock);
    
    if (_head == NULL) {
        pthread_mutex_unlock(_lock);
        return false;
    }
    
    if (_head->val == val) {
        Node<T>* tmp = _head->next;
        delete _head;
        _head = tmp;
        _length--;
        pthread_mutex_unlock(_lock);
        return true;
    }
    
    Node<T>* prev = _head;
    Node<T>* cur = prev->next;
    while (cur != NULL) {
        if (val == cur->val) {
            prev->next = cur->next;
            delete cur;
            _length--;
            pthread_mutex_unlock(_lock);
            return true;
        }
        
        prev = cur;
        cur = cur->next;
    }
    
    pthread_mutex_unlock(_lock);
    return false;
}

template<typename T>
bool CoarseGrainedList<T>::contains(T val)
{
    pthread_mutex_lock(_lock);
    
    Node<T>* cur = _head;
    while (cur != NULL) {
        if (val == cur->val) {
            pthread_mutex_unlock(_lock);
            return true;
        }
        
        cur = cur->next;
    }
    
    pthread_mutex_unlock(_lock);
    return false;
}

template<typename T>
int CoarseGrainedList<T>::length()
{
    return _length;
}

template<typename T>
bool CoarseGrainedList<T>::isEmpty()
{
    return _head == NULL;
}

template<typename T>
void CoarseGrainedList<T>::clear()
{
    pthread_mutex_lock(_lock);
    Node<T>* node = _head;
    _head = NULL;
    _length = 0;
    pthread_mutex_unlock(_lock);
    
    while (node != NULL) {
        Node<T>* prev = node;
        node = node->next;
        delete prev;
    }
    
}

template<typename T>
void CoarseGrainedList<T>::printList()
{
    pthread_mutex_lock(_lock);
    Node<T>* node = _head;
    std::cout << "[";
    
    while (node != NULL) {
        std::cout << node->val << ",";
        node = node->next;
    }
    
    std::cout << "]" << std::endl;
    pthread_mutex_unlock(_lock);
}

template<typename T>
std::string CoarseGrainedList<T>::name()
{
    return "CoarseGrained";
}

template<typename T>
T CoarseGrainedList<T>::operator[](int index)
{
    pthread_mutex_lock(_lock);
    int i = 0;
    Node<T>* node = _head;
    
    while (node != NULL) {
        if (index == i) {
            pthread_mutex_unlock(_lock);
            return node->val;
        }
        i++;
        node = node->next;
    }
    
    pthread_mutex_unlock(_lock);
    throw std::out_of_range("Index exceeds list length.");
}

template class CoarseGrainedList<int>;
template class CoarseGrainedList<double>;
template class CoarseGrainedList<long>;
