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
#include <fstream>
#include <sys/time.h>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"
#include "../src/fine_grained_list.h"
#include "../src/lock_free_list.h"
#include "monitor.h"

// #define DEBUGGING

#ifdef DEBUGGING
#define print_debug_result print_result
#else
#define print_debug_result(...)
#endif

using namespace std;

typedef int k_t;
typedef int v_t;

bool test_sanity(List<k_t, v_t>& list);

int test_size(List<k_t, v_t>& list, int threads, k_t* keys,
     v_t* vals, int num_vals, int expected_size, int max_size);

int test_jam(List<k_t, v_t>& list, int threads, k_t* keys,
     v_t* vals, int num_vals, int expected_size, int max_size);

void run_test_loop(vector<List<k_t, v_t>*>& lists,
    int (*test)(List<k_t,v_t>&, int, k_t*, v_t*, int, int, int),
    int start_val, int end_val, int start_p_2, int end_p_2, int max_len);

void print_result(string test, string list, int vals, int threads, bool passed, double duration, ostringstream* error);
void print_header();

Monitor* monitor = Monitor::getInstance();
bool toCSV = false;
ofstream csvFile;

int main(int argc, char** argv)
{
    char* filenamebase;
    cout << "Running test suite (maximimum threads: " << omp_get_max_threads() << ")" << endl;
    if (argc == 2) {
        filenamebase = argv[1];
        toCSV = true;
        cout << "Results being written to " << filenamebase << "-[pass].csv" << endl;
    } else if (argc > 2) {
        cout << "Usage: ./test <filename>" << endl;
        cout << "Optional filename for output to CSV (given without extension)" << endl;
    }

    vector<List<k_t, v_t>*> lists;
    lists.push_back(new CoarseGrainedList<k_t, v_t>());
    lists.push_back(new FineGrainedList<k_t, v_t>());
    lists.push_back(new LockFreeList<k_t, v_t>());

    bool isSane = true;
    for (unsigned int i = 0; i < lists.size(); i++) {
        isSane &= test_sanity(*lists[i]);
    }
    if (isSane) {
        cout << "Sanity tests passed!" << endl << endl;
    } else {
        cout << "Sanity tests failed! #define DEBUG to run in debug mode." << endl << endl;
    }

    const int passes = 3;

    for (int i = 0; i < passes; i++) {
        cout << " *** " << " Pass " << i << " ***" << endl << endl;
        if (toCSV) {
            ostringstream fname;
            fname << filenamebase << "-" << i << ".csv";
            csvFile.open(fname.str());
        }
        print_header();

        run_test_loop(lists, test_size, 10000, 1000000, 0, 4, -1);
        run_test_loop(lists, test_jam, 1000000, 100000000, 0, 4, 128);
        run_test_loop(lists, test_jam, 1000000, 100000000, 0, 4, 16);

        if (toCSV) {
            csvFile.close();
        } else {
            cout << endl << endl;
        }

    }

    return 0;
}

void run_test_loop(vector<List<k_t,v_t>*>& lists,
    int (*test)(List<k_t, v_t>&, int, k_t*, v_t*, int, int, int),
    int start_val, int end_val, int start_p_2, int end_p_2, int max_len)
{
    for (int i = start_val; i < end_val; i *= 10) {
        k_t* keys = (k_t*)malloc(i * sizeof(k_t));
        v_t* vals = (v_t*)malloc(i * sizeof(v_t));
        for (int j = 0; j < i; j++) {
            keys[j] = rand();
            vals[j] = rand();
        }

        for (int p_2 = start_p_2; p_2 < end_p_2; p_2++) {
            int threads = 1<<p_2;
            int expected = -1;

            for (unsigned int id = 0; id < lists.size(); id++) {
                int result = test(*lists[id], threads, keys, vals, i, expected, max_len);
                if (id == 0) {
                    expected = result;
                }
                lists[id]->clear();
            }
        }
        free(keys);
        free(vals);
    }
}

bool test_sanity(List<k_t, v_t>& list)
{
    const int threads = 1;
    const int iters = 100;

    omp_set_num_threads(threads);

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        list.insert(i, i);
    }

    #pragma omp parallel for
    for (int i = 0; i < iters; i++) {
        v_t x = list[i];
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
        message << "You suck! list.size() was " << list.size();
        print_debug_result("Sanity", list.name(), iters, 1, false, nan(""), &message);
        return false;
    }
}

int test_size(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size)
{
    omp_set_num_threads(threads);

    monitor->start();

    #pragma omp parallel for
    for (int i = 0; i < num_vals; i++) {
        list.insert(keys[i], vals[i]);
    }

    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();

    int size = list.size();
    if (expected_size < 0 || size == expected_size) {
        print_result("Size", list.name(), num_vals, threads, true, elapsed, NULL);
    } else {
        ostringstream message;
        message << "Size was " << size << " (expected " << expected_size << ")";
        print_result("Size", list.name(), num_vals, threads, false, elapsed, &message);
    }
    return size;
}

int test_jam(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size)
{
    omp_set_num_threads(threads);

    monitor->start();

    #pragma omp parallel for
    for (int i = 0; i < num_vals; i++) {
        list.insert(keys[i], vals[i]);
        if (i >= max_size) {
            list.remove(keys[i]);
        }
    }

    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();

    int size = list.size();
    ostringstream label;
    label << "Jam-" << max_size;
    if (expected_size < 0 || size == expected_size) {
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, NULL);
    } else {
        ostringstream message;
        message << "size was " << size << " (expected " << expected_size << ")";
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, &message);
    }
    return size;
}

void print_result(string test, string list, int vals, int threads, bool passed, double duration, ostringstream* error)
{
    if (toCSV) {
        csvFile << test << "," << list << "," << vals << "," << threads << "," << (passed ? "pass" : "FAIL") << "," << setprecision(3) << fixed << duration << endl;
        if (!passed) {
            cerr << error->str() << endl;
        }
    } else {
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
}

void print_header()
{
    if (toCSV) {
        csvFile << "Test,List,Iterations,P,Result,Duration" << endl;
    } else {
        cout
            << left << setfill(' ') << " "
            << setw(18) << "Test"
            << setw(17) << "List"
            << setw(14) << "Iterations"
            << setw(6) << "P"
            << setw(10) << "Result"
            << setw(14) << "Duration (sec)"
            << endl;

    cout << left << setw(80) << setfill('-') << "" << endl;
    }
}
