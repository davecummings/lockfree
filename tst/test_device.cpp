#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstdlib>
#include <ios>
#include <iomanip>
#include <vector>
#include <time.h>
#include <cmath>
#include <fstream>
#include <thread>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"
#include "../src/fine_grained_list.h"
#include "../src/lock_free_list.h"
#include "monitor.h"

#ifdef DEBUGGING
#define print_debug_result print_result
#else
#define print_debug_result(...)
#endif

using namespace std;

/////////// TYPEDEFS ///////////

typedef int k_t;
typedef int v_t;

typedef struct loop_args {
    k_t* keys;
    v_t* vals;
    int start_val;
    int end_val;
    int num_vals;
    List<k_t, v_t>* list;
    int max_size;
} Args;

typedef void* (*test_job)(void*);

/////////// PROTOTYPES ///////////

bool test_sanity(List<k_t, v_t>& list);

int test_size(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size);

int test_jam(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size);

void run_test_loop(vector<List<k_t, v_t>*>& lists,
    int (*test)(List<k_t,v_t>&, int, k_t*, v_t*, int, int, int),
    int start_val, int end_val, int start_p_2, int end_p_2, int max_len);

static void* test_job_insert(void* args_ptr);
static void* test_job_check(void* args_ptr);
static void* test_job_remove(void* args_ptr);
static void* test_job_insert_remove(void* args_ptr);

Args get_args(List<k_t, v_t>* list, int threads, int thread_id, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size);

void print_result(string test, string list, int vals, int threads, bool passed,
    double duration, double powerCPU, double powerDDR, double powerSSD, ostringstream* error);

void print_header();

/////////// GLOBALS ///////////

Monitor* monitor = Monitor::getInstance();
bool toCSV = false;
ofstream csvFile;

/////////// FUNCTIONS /////////

int main(int argc, char** argv)
{
    char* filenamebase;
    cout << "Running test suite (maximimum threads: " << std::thread::hardware_concurrency() << ")" << endl;
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

void run_test_loop(vector<List<k_t, v_t>*>& lists,
    int (*test)(List<k_t,v_t>&, int, k_t*, v_t*, int, int, int),
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
    
    k_t keys[iters];
    v_t vals[iters];
    for (int i = 0; i < iters; i++) {
        keys[i] = i;
        vals[i] = i;
    }
    
    pthread_t pthreads[threads];
    Args argses[threads];
    const int num_loops = 3;
    static test_job loops[num_loops] = { &test_job_insert, &test_job_check, &test_job_remove };
    
    for (int loop_id = 0; loop_id < num_loops; loop_id++) {
        for (int i = 0; i < threads; i++) {
            argses[i] = get_args(&list, threads, i, keys, vals, iters, -1, iters);
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
        message << "You suck! list.size() was " << list.size();
        print_debug_result("Sanity", list.name(), iters, 1, false, nan(""), nan(""), &message);
        return false;
    }
}

int test_size(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size)
{
    pthread_t pthreads[threads];
    Args argses[threads];
    
    monitor->setSampleGranularity(10);
    monitor->start(true);
    
    for (int i = 0; i < threads; i++) {
        argses[i] = get_args(&list, threads, i, keys, vals, num_vals, expected_size, max_size);
        pthread_create(&pthreads[i], NULL, &test_job_insert, &argses[i]);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(pthreads[i], NULL);
    }
    
    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();
    
    int size = list.size();
    if (expected_size < 0 || size == expected_size) {
        print_result("Size", list.name(), num_vals, threads, true, elapsed,
            monitor->getCPUPowerUsage(), monitor->getDDRPowerUsage(), monitor->getSSDPowerUsage(), NULL);
    } else {
        ostringstream message;
        message << "Size was " << size << " (expected " << expected_size << ")";
        print_result("Size", list.name(), num_vals, threads, false, elapsed,
            monitor->getCPUPowerUsage(), monitor->getDDRPowerUsage(), monitor->getSSDPowerUsage(), &message);
    }
    return size;
}

int test_jam(List<k_t, v_t>& list, int threads, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size)
{
    pthread_t pthreads[threads];
    Args argses[threads];
    
    monitor->start(true);
    
    for (int i = 0; i < threads; i++) {
        argses[i] = get_args(&list, threads, i, keys, vals, num_vals, expected_size, max_size);
        pthread_create(&pthreads[i], NULL, &test_job_insert_remove, &argses[i]);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(pthreads[i], NULL);
    }
    
    monitor->stop();
    double elapsed = monitor->getElapsedSeconds();
    
    int size = list.size();
    ostringstream label;
    label << "Jam-" << max_size;
    if (expected_size < 0 || size == expected_size) {
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed,
            monitor->getCPUPowerUsage(), monitor->getDDRPowerUsage(), monitor->getSSDPowerUsage(), NULL);
    } else {
        ostringstream message;
        message << "size was " << size << " (expected " << expected_size << ")";
        print_result(label.str(), list.name(), num_vals, threads, true, elapsed,
            monitor->getCPUPowerUsage(), monitor->getDDRPowerUsage(), monitor->getSSDPowerUsage(), &message);
    }
    return size;
}

static void* test_job_insert(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    
    for (int i = args->start_val; i < args->end_val; i++) {
        args->list->insert(args->vals[i], args->vals[i]);
    }
    
    return NULL;
}
static void* test_job_check(void* args_ptr)
{
    Args* args = static_cast<Args*>(args_ptr);
    for (int i = args->start_val; i < args->end_val; i++) {
        v_t x = args->list->operator[](i);
        if (x != i) {
            ostringstream message;
            message << "You suck! list[" << i << "] was " << x << ")";
            print_debug_result("Sanity", args->list->name(), args->max_size, 1, false, nan(""), nan(""), &message);
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
        args->list->insert(args->vals[i], args->vals[i]);
        if (i >= args->max_size) {
            args->list->remove(args->vals[i]);
        }
    }
    return NULL;
}

Args get_args(List<k_t, v_t>* list, int threads, int thread_id, k_t* keys, v_t* vals, int num_vals, int expected_size, int max_size)
{
    Args args;
    int chunk = num_vals / threads;
    args.keys = keys;
    args.vals = vals;
    args.start_val = chunk * thread_id;
    args.list = list;
    args.max_size = max_size;
    args.num_vals = num_vals;

    if (thread_id + 1 == threads) {
        args.end_val = num_vals;
    } else {
        args.end_val = chunk * (thread_id+1);
    }
    
    return args;
}

void print_result(string test, string list, int vals, int threads, bool passed,
    double duration, double powerCPU, double powerDDR, double powerSSD, ostringstream* error)
{
    if (toCSV) {
        csvFile << test << "," << list << "," << vals << "," << threads << "," << (passed ? "pass" : "FAIL") << "," << setprecision(3) << fixed << duration << "," << setprecision(3) << fixed << powerCPU << "," << setprecision(3) << fixed << powerDDR << "," << setprecision(3) << fixed << powerSSD << endl;
        if (!passed) {
            cerr << error->str() << endl;
        }
    } else {
        double power = powerCPU + powerDDR + powerSSD;
        cout
            << left << setfill(' ') << " "
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
}

void print_header()
{
    if (toCSV) {
        csvFile << "Test,List,Iters,P,Result,Sec,mWh-CPU,mWh-RAM,mWh-SSD" << endl;
    } else {
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
}
