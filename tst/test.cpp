#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"

using namespace std;

typedef int elem_t;

void test_sanity(List<elem_t>& list);
void test_length(List<elem_t>& list, int threads, int num_nums);
void test_pressure(List<elem_t>& list, int threads, int num_nums);

int main()
{
    cout << "Running test suite (maximimum threads: " << omp_get_max_threads() << ")" << endl;

    CoarseGrainedList<elem_t>* cList = new CoarseGrainedList<elem_t>();
    test_sanity(*cList);

    for (int j = 100; j < 1000000; j *= 10) {
        for (int i = 0; i < 4; i++) {
            test_length(*cList, 1<<i, j);
            delete cList;
            cList = new CoarseGrainedList<elem_t>();
        }
    }

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

    int* nums = (int*)malloc(num_nums * sizeof(int));

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
        printf("Random test on %d numbers with %d threads passed! (%.3f seconds)\n", num_nums, threads, elapsed);
    } else {
        printf("Random test on %d numbers with %d threads failed! (expected %d, actual %d)\n", num_nums, threads, num_nums, length);
    }
}

void test_pressure(List<elem_t>& list, int threads, int num_nums)
{
    const int max_len = 50;

    omp_set_num_threads(threads);

    int* nums = (int*)malloc(num_nums * sizeof(int));

    for (int i = 0; i < num_nums; i++) {
        nums[i] = rand();
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma omp parallel for
    for (int i = 0; i < num_nums; i++) {
        list.insert(nums[i]);
        if (i >= max_len) {
            list.remove(nums[i-max_len]);
        }
    }

    gettimeofday(&end, NULL);

    double elapsed = ((double)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))) / 1000000.0f;

    int length = list.length();
    if (length == max_len) {
        printf("Random test on %d numbers with %d threads passed! (%.3f seconds)\n", num_nums, threads, elapsed);
    } else {
        printf("Random test on %d numbers with %d threads failed! (expected %d, actual %d)\n", num_nums, threads, max_len, length);
    }
}