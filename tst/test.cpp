#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstdlib>
#include <ios>
#include <iomanip>
#include <vector>
#include <omp.h>
#include <time.h>
#include <cmath>
#include <sys/time.h>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"
#include "../src/fine_grained_list.h"
#include "../src/lock_free_list.h"

// #define DEBUG

#ifdef DEBUG
#define print_debug_result print_result
#else
#define print_debug_result(...)
#endif

using namespace std;

typedef long elem_t;

bool test_sanity(List<elem_t>& list);

int test_length(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length);

int test_pressure(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length);

void run_test_loop(vector<List<elem_t>*>& lists,
    int (*test)(List<elem_t>&, int, elem_t*, int, int, int),
    int start_val, int end_val, int start_p_2, int end_p_2, int max_len);

void print_result(string test, string list, int vals, int threads, bool passed, double duration, ostringstream* error);

void print_header();

int main()
{
    cout << "Running test suite (maximimum threads: " << omp_get_max_threads() << ")" << endl;

    vector<List<elem_t>*> lists;
    lists.push_back(new CoarseGrainedList<elem_t>());
    lists.push_back(new FineGrainedList<elem_t>());
    lists.push_back(new LockFreeList<elem_t>());

    bool isSane = true;
    for (unsigned int i = 0; i < lists.size(); i++) {
        isSane &= test_sanity(*lists[i]);
    }
    if (isSane) {
        cout << "Sanity tests passed!" << endl << endl;
    } else {
        cout << "Sanity tests failed! #define DEBUG to run in debug mode." << endl << endl;
    }

    string passes[] = { "1st", "2nd", "3rd" };

    for (int i = 0; i < 3; i++) {
        cout << " *** " << passes[i] << " pass *** " << endl << endl;
        print_header();

        run_test_loop(lists, test_length, 10000, 1000000, 0, 4, -1);
        run_test_loop(lists, test_pressure, 1000000, 100000000, 0, 4, 128);
        run_test_loop(lists, test_pressure, 1000000, 100000000, 0, 4, 16);

        cout << endl << endl;
    }

    return 0;
}

void run_test_loop(vector<List<elem_t>*>& lists,
    int (*test)(List<elem_t>&, int, elem_t*, int, int, int),
    int start_val, int end_val, int start_p_2, int end_p_2, int max_len)
{
    for (int i = start_val; i < end_val; i *= 10) {
        elem_t* vals = (elem_t*)malloc(i * sizeof(elem_t));
        for (int j = 0; j < i; j++) {
            vals[j] = rand();
        }

        for (int p_2 = start_p_2; p_2 < end_p_2; p_2++) {
            int threads = 1<<p_2;
            int expected = -1;

            for (unsigned int id = 0; id < lists.size(); id++) {
                int result = test(*lists[id], threads, vals, i, expected, max_len);
                if (id == 0) {
                    expected = result;
                }
                lists[id]->clear();
            }
        }
        free(vals);
    }
}

bool test_sanity(List<elem_t>& list)
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
            ostringstream message;
            message << "You suck! list[" << i << "] was " << x << ")";
            print_debug_result("Sanity", list.name(), iters, 1, false, nan(""), &message);
        }
    }

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        list.remove(i);
    }

    if (list.isEmpty()) {
        print_debug_result("Sanity", list.name(), iters, 1, true, nan(""), NULL);
        return true;
    } else {
        ostringstream message;
        message << "You suck! list.length() was " << list.length();
        print_debug_result("Sanity", list.name(), iters, 1, false, nan(""), &message);
        return false;
    }
}

int test_length(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length)
{
    omp_set_num_threads(threads);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma omp parallel for
    for (int i = 0; i < num_vals; i++) {
        list.insert(vals[i]);
    }

    gettimeofday(&end, NULL);

    double elapsed = ((double)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))) / 1000000.0f;

    int length = list.length();
    if (expected_length < 0 || length == expected_length) {
        print_result("Length", list.name(), num_vals, threads, true, elapsed, NULL);
    } else {
        ostringstream message;
        message << "length was " << length << " (expected " << expected_length << ")";
        print_result("Length", list.name(), num_vals, threads, false, elapsed, &message);
    }
    return length;
}

int test_pressure(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length)
{
    omp_set_num_threads(threads);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    #pragma omp parallel for
    for (int i = 0; i < num_vals; i++) {
        list.insert(vals[i]);
        if (i >= max_length) {
            list.remove(vals[i]);
        }
    }

    gettimeofday(&end, NULL);

    double elapsed = ((double)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))) / 1000000.0f;

    int length = list.length();
    ostringstream label;
    label << "Pressure-" << max_length;
    if (expected_length < 0 || length == expected_length) {
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, NULL);
    } else {
        ostringstream message;
        message << "length was " << length << " (expected " << expected_length << ")";
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, &message);
    }
    return length;
}

void print_result(string test, string list, int vals, int threads, bool passed, double duration, ostringstream* error)
{
    cout << left << setfill(' ') << " "
        << setw(21) << test
        << setw(17) << list
        << setw(11) << vals 
        << setw(6) << threads
        << setw(10) << (passed ? "pass" : "FAIL")
        << setw(14) << setprecision(3) << fixed << duration;

    if (!passed && error != NULL) {
        cout << " " << error->str();
    }

    cout << endl;
}

void print_header()
{
    cout
        << left << setfill(' ') << " "
        << setw(18) << "Test"
        << setw(17) << "List"
        << setw(14) << "iterations"
        << setw(6) << "P"
        << setw(10) << "Result"
        << setw(14) << "Duration (sec)"
        << endl;

    cout << left << setw(80) << setfill('-') << "" << endl;
}
