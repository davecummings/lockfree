#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"
#include "../src/fine_grained_list.h"
#include "../src/lock_free_list.h"

using namespace std;

typedef long elem_t;

void test_sanity(List<elem_t>& list);
void test_length(List<elem_t>& list, int threads, int num_nums);
void test_pressure(List<elem_t>& list, int threads, int num_nums);

int main()
{
    cout << "Running test suite (maximimum threads: " << omp_get_max_threads() << ")" << endl;

    cout << "***COARSE***" << endl;

    CoarseGrainedList<elem_t>* cList = new CoarseGrainedList<elem_t>();
    test_sanity(*cList);

    // for (int j = 10000; j < 1000000; j *= 10) {
    //     for (int i = 0; i < 4; i++) {
    //         test_length(*cList, 1<<i, j);
    //         delete cList;
    //         cList = new CoarseGrainedList<elem_t>();
    //         test_pressure(*cList, 1<<i, j);
    //         delete cList;
    //         cList = new CoarseGrainedList<elem_t>();
    //     }
    // }

    cout << "***FINE***" << endl;

    FineGrainedList<elem_t>* fList = new FineGrainedList<elem_t>();
    test_sanity(*fList);

    for (int j = 1000; j < 10000; j *= 10) {
        for (int i = 0; i < 4; i++) {
            // test_length(*fList, 1<<i, j);
            // delete fList;
            // fList = new FineGrainedList<elem_t>();
            test_pressure(*fList, 1<<i, j);
            delete fList;
            fList = new FineGrainedList<elem_t>();
        }
    }

    // cout << "***LOCK-FREE***" << endl;

    // LockFreeList<elem_t>* lfList = new LockFreeList<elem_t>();
    // test_sanity(*lfList);

    // lfList->printList();

    // for (int j = 10000; j < 1000000; j *= 10) {
    //     for (int i = 0; i < 4; i++) {
    //         test_length(*lfList, 1<<i, j);
    //         delete lfList;
    //         lfList = new LockFreeList<elem_t>();
    //         test_pressure(*lfList, 1<<i, j);
    //         delete lfList;
    //         lfList = new LockFreeList<elem_t>();
    //     }
    // }

    return 0;
}

void test_sanity(List<elem_t>& list)
{
    omp_set_num_threads(1);
    const int iters = 100;

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        list.insert(i);
    }

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        elem_t x = list[i];
        if (x != i) {
            cout << "Sanity test failed! You suck! (list[" << i << "] was " << x << ")" << endl;
        }
    }

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        list.remove(i);
    }

    if (list.isEmpty()) {
        cout << "Sanity test passed!" << endl;
    } else {
        cout << "Sanity test failed! You suck!" << endl;
    }
}

void test_length(List<elem_t>& list, int threads, int num_nums)
{
    omp_set_num_threads(threads);

    elem_t* nums = (elem_t*)malloc(num_nums * sizeof(elem_t));

    for (int i = 0; i < num_nums; i++) {
        nums[i] = rand();
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma omp parallel for
    for (int i = 0; i < num_nums; i++) {
        list.insert(nums[i]);
    }

    gettimeofday(&end, NULL);

    double elapsed = ((double)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))) / 1000000.0f;

    int length = list.length();
    if (length == num_nums) {
        printf("Length test on %d numbers with %d threads passed! (%.3f seconds)\n", num_nums, threads, elapsed);
    } else {
        printf("Length test on %d numbers with %d threads failed! (expected %d, actual %d)\n", num_nums, threads, num_nums, length);
        CoarseGrainedList<elem_t> expected = *(new CoarseGrainedList<elem_t>());
        for (int i = 0; i < num_nums; i++) {
            expected.insert(nums[i]);
        }

        for (int i = 0; i < num_nums; i++) {
            if (list[i] != expected[i]) {
                cout << "list[" << i << "] = " << list[i] << " (expected " << expected[i] << ")" << endl;
            }
        }
    }
}

void test_pressure(List<elem_t>& list, int threads, int num_nums)
{
    const int max_len = 50;

    omp_set_num_threads(threads);

    elem_t* nums = (elem_t*)malloc(num_nums * sizeof(elem_t));

    for (int i = 0; i < num_nums; i++) {
        nums[i] = i;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma omp parallel for
    for (int i = 0; i < num_nums; i++) {
        list.insert(nums[i]);
        if (i >= max_len) {
            list.remove(nums[i]);
            
        }
    }

    gettimeofday(&end, NULL);

    double elapsed = ((double)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))) / 1000000.0f;

    int length = list.length();
    if (length == max_len) {
        printf("Pressure test on %d numbers with %d threads passed! (%.3f seconds)\n", num_nums, threads, elapsed);
    } else {
        printf("Pressure test on %d numbers with %d threads failed! (expected %d, actual %d)\n", num_nums, threads, max_len, length);
    }
}
