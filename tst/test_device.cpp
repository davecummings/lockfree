#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstdlib>
#include <ios>
#include <iomanip>
#include <vector>
#include <time.h>
#include <cmath>
#include <thread>

#include "list.h"
#include "coarse_grained_list.h"
#include "fine_grained_list.h"
#include "lock_free_list.h"
#include "monitor.h"

#ifdef DEBUGGING
#define print_debug_result print_result
#else
#define print_debug_result(...)
#endif

using namespace std;

/////////// TYPEDEFS ///////////

typedef long elem_t;

typedef struct loop_args {
    elem_t* vals;
    int start_val;
    int end_val;
    int num_vals;
    List<elem_t>* list;
    int max_length;
} Args;

typedef void* (*test_job)(void*);

/////////// PROTOTYPES ///////////

bool test_sanity(List<elem_t>& list);

int test_length(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length);

int test_jam(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length);

void run_test_loop(vector<List<elem_t>*>& lists,
                   int (*test)(List<elem_t>&, int, elem_t*, int, int, int),
                   int start_val, int end_val, int start_p_2, int end_p_2, int max_length);

static void* test_job_insert(void* args_ptr);
static void* test_job_check(void* args_ptr);
static void* test_job_remove(void* args_ptr);
static void* test_job_insert_remove(void* args_ptr);

Args get_args(List<elem_t>* list, int num_vals, int threads, int thread_id, elem_t* vals, int max_length);

void print_result(string test, string list, int vals, int threads, bool passed, double duration, double power, ostringstream* error);

void print_header();

/////////// GLOBALS ///////////

Monitor* monitor = Monitor::getInstance();

/////////// FUNCTIONS /////////

int main()
{
    cout << "Running test suite (maximimum threads: " << std::thread::hardware_concurrency() << ")" << endl;
    
    vector<List<elem_t>*> lists;
//    lists.push_back(new CoarseGrainedList<elem_t>());
//    lists.push_back(new FineGrainedList<elem_t>());
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
    
    const int passes = 3;
    
    for (int i = 0; i < passes; i++) {
        cout << " *** " << " Pass " << i << " ***" << endl << endl;
        print_header();
        
        run_test_loop(lists, test_length, 10000, 1000000, 0, 4, -1);
        run_test_loop(lists, test_jam, 1000000, 100000000, 0, 4, 128);
        run_test_loop(lists, test_jam, 1000000, 100000000, 0, 4, 16);
        
        sleep(1000);
        
        cout << endl << endl;
    }
    
    return 0;
}

void run_test_loop(vector<List<elem_t>*>& lists,
                   int (*test)(List<elem_t>&, int, elem_t*, int, int, int),
                   int start_val, int end_val, int start_p_2, int end_p_2, int max_length)
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
                int result = test(*lists[id], threads, vals, i, expected, max_length);
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
    const int threads = 1;
    const int iters = 100;
    
    elem_t vals[iters];
    for (int i = 0; i < iters; i++) {
        vals[i] = i;
    }
    
    pthread_t pthreads[threads];
    Args argses[threads];
    const int num_loops = 3;
    static test_job loops[num_loops] = { &test_job_insert, &test_job_check, &test_job_remove };
    
    for (int loop_id = 0; loop_id < num_loops; loop_id++) {
        for (int i = 0; i < threads; i++) {
            argses[i] = get_args(&list, iters, threads, i, vals, -1);
            pthread_create(&pthreads[i], NULL, loops[loop_id], &argses[i]);
        }
        for (int i = 0; i < threads; i++) {
            pthread_join(pthreads[i], NULL);
        }
    }
    
    if (list.isEmpty()) {
        print_debug_result("Sanity", list.name(), iters, 1, true, nan(""), nan(""), NULL);
        return true;
    } else {
        ostringstream message;
        message << "You suck! list.length() was " << list.length();
        print_debug_result("Sanity", list.name(), iters, 1, false, nan(""), nan(""), &message);
        return false;
    }
}

int test_length(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length)
{
    pthread_t pthreads[threads];
    Args argses[threads];
    
    monitor->setSampleGranularity(10);
    monitor->start(true);
    
    for (int i = 0; i < threads; i++) {
        argses[i] = get_args(&list, num_vals, threads, i, vals, -1);
        pthread_create(&pthreads[i], NULL, &test_job_insert, &argses[i]);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(pthreads[i], NULL);
    }
    
    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();
    double power = monitor->getPowerUsage();
    
    int length = list.length();
    if (expected_length < 0 || length == expected_length) {
        print_result("Length", list.name(), num_vals, threads, true, elapsed, power, NULL);
    } else {
        ostringstream message;
        message << "length was " << length << " (expected " << expected_length << ")";
        print_result("Length", list.name(), num_vals, threads, false, elapsed, power, &message);
    }
    return length;
}

int test_jam(List<elem_t>& list, int threads, elem_t* vals, int num_vals, int expected_length, int max_length)
{
    pthread_t pthreads[threads];
    Args argses[threads];
    
    monitor->start(true);
    
    for (int i = 0; i < threads; i++) {
        argses[i] = get_args(&list, num_vals, threads, i, vals, -1);
        pthread_create(&pthreads[i], NULL, &test_job_insert_remove, &argses[i]);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(pthreads[i], NULL);
    }
    
    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();
    double power = monitor->getDDRPowerUsage();
    
    int length = list.length();
    ostringstream label;
    label << "Jam-" << max_length;
    if (expected_length < 0 || length == expected_length) {
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, power, NULL);
    } else {
        ostringstream message;
        message << "length was " << length << " (expected " << expected_length << ")";
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed, power, &message);
    }
    return length;
}

static void* test_job_insert(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    
    for (int i = args->start_val; i < args->end_val; i++) {
        args->list->insert(args->vals[i]);
    }
    
    return NULL;
}
static void* test_job_check(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    for (int i = args->start_val; i < args->end_val; i++) {
        elem_t x = args->list->operator[](i);
        if (x != i) {
            ostringstream message;
            message << "You suck! list[" << i << "] was " << x << ")";
            print_debug_result("Sanity", args->list->name(), args->max_length, 1, false, nan(""), nan(""), &message);
        }
    }
    return NULL;
}
static void* test_job_remove(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    for (int i = args->start_val; i < args->end_val; i++) {
        args->list->remove(args->vals[i]);
    }
    return NULL;
}
static void* test_job_insert_remove(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    for (int i = args->start_val; i < args->end_val; i++) {
        args->list->insert(args->vals[i]);
        if (i >= args->max_length) {
            args->list->remove(args->vals[i]);
        }
    }
    return NULL;
}

Args get_args(List<elem_t>* list, int num_vals, int threads, int thread_id, elem_t* vals, int max_length)
{
    Args args;
    int chunk = num_vals / threads;
    args.vals = vals;
    args.start_val = chunk * thread_id;
    args.list = list;
    args.max_length = max_length;
    args.num_vals = num_vals;

    if (thread_id + 1 == threads) {
        args.end_val = num_vals;
    } else {
        args.end_val = chunk * (thread_id+1);
    }
    
    return args;
}

void print_result(string test, string list, int vals, int threads, bool passed, double duration, double power, ostringstream* error)
{
    cout << left << setfill(' ') << " "
    << setw(10) << test
    << setw(17) << list
    << setw(11) << vals
    << setw(6) << threads
    << setw(10) << (passed ? "pass" : "FAIL")
    << setw(10) << setprecision(3) << fixed << duration
    << setw(10) << setprecision(3) << fixed << power
    << setw(5) << setprecision(1) << fixed << power / duration * 3.6;
    
    if (!passed && error != NULL) {
        cout << " " << error->str();
    }
    
    cout << endl;
}

void print_header()
{
    cout
    << left << setfill(' ') << " "
    << setw(10) << "Test"
    << setw(17) << "List"
    << setw(11) << "Iters"
    << setw(6) << "P"
    << setw(10) << "Result"
    << setw(10) << "Sec"
    << setw(10) << "mWh"
    << setw(5) << "W"
    << endl;
    
    cout << left << setw(80) << setfill('-') << "" << endl;
}
