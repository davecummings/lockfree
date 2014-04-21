#include <stdexcept>

#include "course_grained_list.h"

using namespace std;

CourseGrainedList::CourseGrainedList() : List()
{
	_head = NULL;
    length = 0;
    _lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

CourseGrainedList::~CourseGrainedList()
{
    pthread_mutex_lock(_lock);

	Node* node = _head;
	while (node != NULL)
    {
		Node* tmp = node;
		node = node->next;
		delete tmp;
	}

    pthread_mutex_unlock(_lock);
	pthread_mutex_destroy(_lock);
    free(_lock);
}

bool CourseGrainedList::insert(T val)
{
	Node* node = new Node;
	if (node == NULL) {
		return false;
	}
	node->val = val;

	pthread_mutex_lock(_lock);

	if (_head == NULL) {
		node->next = NULL;
		_head = node;
	} else if (val < _head->val) {
		node->next = _head;
		_head = node;

	} else {
		Node* cur = _head->next;
		Node* prev = _head;

		while (cur != NULL) {
			if (val < cur->val) {
				break;
			}
			prev = cur;
			cur = cur->next;
		}

		prev->next = node;
		node->next = cur;
	}

    length++;
	pthread_mutex_unlock(_lock);
	return true;
}

bool CourseGrainedList::remove(T val)
{
    pthread_mutex_lock(_lock);

    if (_head == NULL) {
        pthread_mutex_unlock(_lock);
        return false;
    }

    if (_head->val == val) {
        Node* tmp = _head->next;
        delete _head;
        _head = tmp;
        length--;
        pthread_mutex_unlock(_lock);
        return true;
    }

    Node* prev = _head;
    Node* cur = prev->next;
    while (cur != NULL) {
        if (val == cur->val) {
            prev->next = cur->next;
            delete cur;
            length--;
            pthread_mutex_unlock(_lock);
            return true;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    return false;
}

bool CourseGrainedList::contains(T val)
{
    pthread_mutex_lock(_lock);

    Node* cur = _head;
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

int CourseGrainedList::length()
{
    pthread_mutex_lock(_lock);
	int length = 0;
	Node* node = _head;

    while (node != NULL) {
    	length++;
    	node = node->next;
    }

    pthread_mutex_unlock(_lock);

    return length;
}

T CourseGrainedList::operator[](int index)
{
    pthread_mutex_lock(_lock);
    int i = 0;
    Node* node = _head;

    while (node != NULL) {
        if (index == i) {
            pthread_mutex_unlock(_lock);
            return node->val;
        }
        i++;
        node = node->next;
    }

    throw out_of_range("No such element in list.");
}