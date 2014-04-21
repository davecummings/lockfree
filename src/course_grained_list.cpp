#include <stdexcept>

#include "course_grained_list.h"

using namespace std;

CourseGrainedList::CourseGrainedList() : List()
{
	_head = NULL;
    _lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(_lock, NULL);
}

CourseGrainedList::~CourseGrainedList()
{

	Node* node = _head;
	while (node != NULL) {
		Node* tmp = node;
		node = node->next;
		delete tmp;
	}
	pthread_mutex_destroy(_lock);
    free(_lock);
}

bool CourseGrainedList::insert(elem_t val)
{
	Node* node = (Node*) malloc(sizeof(Node));
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

	pthread_mutex_unlock(_lock);
	return true;
}

bool CourseGrainedList::remove(elem_t val)
{
    pthread_mutex_lock(_lock);

    if (_head == NULL) {
        pthread_mutex_unlock(_lock);
        return false;
    }

    if (_head->val == val) {
        Node* tmp = _head->next;
        free(_head);
        _head = tmp;
        pthread_mutex_unlock(_lock);
        return true;
    }

    Node* prev = _head;
    Node* cur = prev->next;
    while (cur != NULL) {
        if (val == cur->val) {
            prev->next = cur->next;
            free(cur);
            pthread_mutex_unlock(_lock);
            return true;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(_lock);
    return false;
}

bool CourseGrainedList::contains(elem_t val)
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

elem_t CourseGrainedList::operator[](int index)
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