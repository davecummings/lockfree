#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "lockfree.h"


List* new_list()
{
    List* l = malloc(sizeof(List));
    if (l == NULL) return NULL;
    l->head = malloc(sizeof(Node));
    l->tail = malloc(sizeof(Node));
    if (l->head == NULL || l->tail == NULL) return NULL;
    l->head->next = l->tail;
    return l;
}

/* We can use the low-order bit because the Node strcut
    is at least five bytes */
Node* get_marked_reference(Node* n)
{
    long int p = (long int) n;
    p = p | 1;
    return (Node*) p;
}

Node* get_unmarked_reference(Node* n)
{
    long int p = (long int) n;
    p = (p >> 1) << 1;
    return (Node*) p;
}

bool is_marked_reference(Node* n)
{
    long int p = (long int) n;
    p = p & 1;
    return (Node*) p;
}

Node* new_node(elem_t val)
{
    Node* n = malloc(sizeof(struct Node));
    if (n == NULL) return NULL;
    n->val = val;
    return n;
}

void search(List* l, elem_t val, Node** left_node, Node** right_node)
{
    Node* left_node_next;

    while (true)
    {
        Node* prev = l->head;
        Node* cur = prev->next;

        /* 1: Find left_node and right_node */
        do
        {
            if (!is_marked_reference(cur))
            {
                *left_node = prev;
                left_node_next = cur;
            }
            prev = get_unmarked_reference(cur);
            if (prev == l->tail) break;
            cur = prev->next;
        } while (is_marked_reference(cur) || prev->val < val);
        *right_node = prev;

        /* 2: Check nodes are adjacent */
        if (left_node_next == *right_node)
        {
            if (*right_node != l->tail && is_marked_reference((*right_node)->next))
                continue;
            else
                return;
        }

        /* 3: Remove one or more marked nodes */
        if (__sync_bool_compare_and_swap(&((*left_node)->next),
            left_node_next, *right_node))
        {
            // right node got marked while performing operation
            if (*right_node != l->tail && is_marked_reference((*right_node)->next))
                continue;
            else
                return;
        }
    }
}

bool insert(List* l, elem_t val)
{
    Node* n = new_node(val);
    Node* left_node;
    Node* right_node;

    while (true)
    {
        search(l, val, &left_node, &right_node);
        // node is already in the list
        if (right_node != l->tail && right_node->val == val)
            return false;
        n->next = right_node;
        if (__sync_bool_compare_and_swap(&left_node->next, right_node, n))
            return true;
    }
}

bool delete(List* l, elem_t val)
{
    Node* left_node;
    Node* right_node;
    Node* right_node_next;

    while (true)
    {
        search(l, val, &left_node, &right_node);
        // node not in list
        if (right_node == l->tail || right_node->val != val)
            return false;
        right_node_next = right_node->next;
        // if it's not already marked for deletion, mark it for deletion
        if (!is_marked_reference(right_node_next))
        {
            if (__sync_bool_compare_and_swap(&right_node->next,
                right_node_next, get_marked_reference(right_node_next)))
                break;
        }
    }

    /* Try to physically delete the node here.
        Otherwise, call search to delete it. */
    if (!__sync_bool_compare_and_swap(&left_node->next,
        right_node, right_node_next))
        search(l, right_node->val, &left_node, &right_node);

    return true;
}

bool find(List* l, elem_t val)
{
    Node* left_node;
    Node* right_node;

    search(l, val, &left_node, &right_node);
    if (right_node == l->tail || right_node->val != val)
        return false;
    else
        return true;
}

// possibly not thread-safe
void print_list(List* l)
{
    printf("[");
    Node* cur = l->head;

    while (true)
    {
        cur = get_unmarked_reference(cur);
        cur = cur->next;
        if (cur == l->tail) break;
        printf("%d,", cur->val);
    }

    printf("]\n");
}
