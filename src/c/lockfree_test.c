#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include "lockfree.h"


static const int OPS_PER_THREAD = 1000000;
static const int MAX_NUM = 200;
static const int THREADS = 4;
static const int OPS = OPS_PER_THREAD * THREADS;

int* nums;
List* list;

void* thread(void* vargp)
{
    int* i = (int*) vargp;
    int start = (*i) * OPS_PER_THREAD;
    for (int i = start; i < start + OPS_PER_THREAD; i++)
    {
        if (i % 2 == 0) insert(list, nums[i]);
        else delete(list, nums[i]);
    }
    return NULL;
}

int main()
{
    srand(time(NULL));
    list = new_list();
    nums = malloc(sizeof(int) * OPS);

    for (int i = 0; i < OPS; i++)
    {
        nums[i] = rand() % MAX_NUM;
    }

    for (int i = 0; i < THREADS; i++)
    {
        pthread_t tid;
        int* n = malloc(sizeof(int));
        *n = i;
        pthread_create(&tid, NULL, thread, n);
        pthread_join(tid, NULL);
    }

    print_list(list);

    return 0;
}
