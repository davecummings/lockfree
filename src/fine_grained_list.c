#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>


typedef int elem_t;

typedef struct Node
{
    elem_t val;
    struct Node* next;
    pthread_mutex_t lock;
} Node;

typedef struct List
{
    Node* head;
    pthread_mutex_t lock; // a lock for the head of the list
} List;


List* new_list()
{
    List* l = malloc(sizeof(List));
    l->head = NULL;
    pthread_mutex_init(&l->lock, NULL);
    return l;
}

bool insert(List* l, elem_t val)
{
    Node* n = malloc(sizeof(Node));
    if (n == NULL) return false;
    n->val = val;
    pthread_mutex_init(&n->lock, NULL);

    pthread_mutex_lock(&l->lock);
    if (l->head == NULL)
    {
        n->next = NULL;
        l->head = n;
        pthread_mutex_unlock(&l->lock);
    }
    else if (val <= l->head->val)
    {
        n->next = l->head;
        l->head = n;
        pthread_mutex_unlock(&l->lock);
    }
    else
    {
        Node* prev = l->head;
        Node* cur = prev->next;
        pthread_mutex_lock(&prev->lock);
        if (cur != NULL) pthread_mutex_lock(&cur->lock);
        pthread_mutex_unlock(&l->lock);

        while (cur != NULL)
        {
            if (val <= cur->val)
                break;

            Node* tmp = prev;
            prev = cur;
            cur = cur->next;
            if (cur != NULL) pthread_mutex_lock(&cur->lock);
            pthread_mutex_unlock(&tmp->lock);
        }

        prev->next = n;
        n->next = cur;

        pthread_mutex_unlock(&prev->lock);
        if (cur != NULL) pthread_mutex_unlock(&cur->lock);
    }

    return true;
}

bool delete(List* l, elem_t val)
{
    pthread_mutex_lock(&l->lock);

    if (l->head == NULL)
    {
        pthread_mutex_unlock(&l->lock);
        return false;
    }

    if (l->head->val == val)
    {
        Node* tmp = l->head->next;
        free(l->head);
        l->head = tmp;
        pthread_mutex_unlock(&l->lock);
        return true;
    }

    Node* prev = l->head;
    Node* cur = prev->next;

    pthread_mutex_lock(&prev->lock);
    if (cur != NULL) pthread_mutex_lock(&cur->lock);
    pthread_mutex_unlock(&l->lock);

    while (cur != NULL)
    {
        if (val == cur->val)
        {
            prev->next = cur->next;
            pthread_mutex_unlock(&prev->lock);
            pthread_mutex_unlock(&cur->lock);
            pthread_mutex_destroy(&cur->lock);
            free(cur);
            return true;
        }

        Node* tmp = prev;
        prev = cur;
        cur = cur->next;
        if (cur != NULL) pthread_mutex_lock(&cur->lock);
        pthread_mutex_unlock(&tmp->lock);
    }

    pthread_mutex_unlock(&prev->lock);
    return false;
}

bool find(List* l, elem_t val)
{
    pthread_mutex_lock(&l->lock);

    Node* cur = l->head;
    if (cur != NULL) pthread_mutex_lock(&cur->lock);
    pthread_mutex_unlock(&l->lock);

    while (cur != NULL)
    {
        if (val == cur->val)
        {
            pthread_mutex_unlock(&cur->lock);
            return true;
        }

        Node* tmp = cur;
        cur = cur->next;
        pthread_mutex_unlock(cur);
    }

    return false;
}

void delete_list(List* l)
{
    pthread_mutex_lock(&l->lock);
    Node* cur = l->head;
    l->head = NULL; // break the list
    if (cur != NULL) pthread_mutex_lock(&cur->lock);
    pthread_mutex_unlock(&l->lock);

    while (cur != NULL)
    {
        Node* tmp = cur;
        cur = cur->next;
        pthread_mutex_lock(&cur->lock);
        pthread_mutex_unlock(&tmp->lock);
        free(tmp);
    }

    pthread_mutex_destroy(&l->lock);
    free(l);
}