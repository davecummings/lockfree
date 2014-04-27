#include <stdbool.h>

typedef int elem_t;

typedef struct Node
{
    elem_t val;
    struct Node* next;
} Node;

/* Use dummy head and tail nodes */
typedef struct List
{
    Node* head;
    Node* tail;
} List;


Node* get_marked_reference(Node*);
Node* get_unmarked_reference(Node*);
bool is_marked_reference(Node*);
List* new_list();
Node* new_node(elem_t val);
void search(List*, elem_t, Node**, Node**);
bool insert(List*, elem_t);
bool delete(List*, elem_t);
bool find(List*, elem_t);
void print_list(List*);