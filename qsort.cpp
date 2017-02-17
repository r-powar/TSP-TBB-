// CPSC 4600 / 5600 - Quicksort
//
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#include "tbb/tbb.h"
using namespace tbb;

const int NUM_THREADS = 8;

// Functions for testing and debugging
void check_array(int *a, int size);
void print_array(string name, int *src, int size);

void swap(int &a, int &b){
    int temp = a;
    a = b;
    b = temp;
}

class PrefixSum{
    int *src;
    int *dest;
    int sum;
    
public:
    PrefixSum(int *_src, int *_dest)
    : src(_src), dest(_dest) {sum = 0;}
    
    PrefixSum(PrefixSum &s, split)
    : src(s.src), dest(s.dest) {sum = 0;}
    
    int getSum() const { return sum;}
    
    template<typename Tag>
    void operator()
    (const blocked_range<int> r, Tag tag){
        for(int i = r.begin(); i < r.end(); i++){
            sum += src[i];
            
            if(tag.is_final_scan())
                dest[i] = sum;
        }
    }
    
    void reverse_join(PrefixSum &s){
        sum += s.sum;
    }
    
    void assign(PrefixSum &s){
        sum = s.sum;
    }
    
};
int parallelPartition(int *a, int low, int high, int pivot){
    int size = high - low + 1;
    if(size == 1){
        return low;
    }
    
    int *b = new int [size];
    int *lt = new int [size];
    int *gt = new int [size];
    int *lt2 = new int[size];
    int *gt2 = new int[size];
    
    
    parallel_for(blocked_range<int>(0, size),
                [&](blocked_range<int> r){
                    for(int i = r.begin(); i < r.end(); i++){
                        b[i] = a[low + i];
                        if(b[i] < pivot){
                            lt[i] = 1;
                        }else{
                            lt[i] = 0;
                        }
                        
                        if(b[i] > pivot){
                            gt[i] = 1;
                        }else{
                            gt[i] = 0;
                        }
                    }
                });
    
    
    PrefixSum sum1(lt, lt2);
    parallel_scan(blocked_range<int>(0, size), sum1);
    PrefixSum sum2(gt, gt2);
    parallel_scan(blocked_range<int>(0, size), sum2);
    
    int pivotIndex = low + lt2[size - 1];
    a[pivotIndex] = pivot;
    
    parallel_for(blocked_range<int>(0, size),
                 [&] (blocked_range<int> r){
                     for(int i = r.begin(); i < r.end(); i++){
                         if(b[i] < pivot){
                             a[low + lt2[i] - 1] = b[i];
                         }
                         if(b[i] > pivot){
                             a[pivotIndex + gt2[i]] = b[i];
                         }
                     }
                 });
    
    return pivotIndex;
}

int partition(int *a, int low, int high){
    int pivot = a[high];
    int pIndex = low - 1;
    for(int j = low; j <= high - 1; j++){
        if(a[j] <= pivot){
            pIndex = pIndex + 1;
            swap(a[pIndex], a[j]);
        }
    }
    swap(a[pIndex + 1], a[high]);
    return pIndex + 1;
}

void qsort_seq(int *a, int low, int high)
{
    if(low < high){
        int pivot = partition(a, low, high);
        qsort_seq(a, low, pivot - 1);
        qsort_seq(a, pivot + 1, high);
    }
}

void p_qsort_inplace(int *a, int low, int high, int cutoff)
{
    if((high - low) < cutoff){
        qsort_seq(a, low, high);
    }else{
        int pivot = partition(a, low, high);
        parallel_invoke([&]{p_qsort_inplace(a, low, pivot - 1, cutoff);},
                        [&]{p_qsort_inplace(a, pivot +1, high, cutoff);});
    }
}

void p_qsort_best_span(int *a, int low, int high, int cutoff)
{
    if((high - low) <= cutoff){
        qsort_seq(a, low, high);
    }else{
        int pivotVal = a[high];
        int pivot = parallelPartition(a, low, high, pivotVal);
        parallel_invoke([&]{p_qsort_best_span(a, low, pivot - 1, cutoff);},
                        [&]{p_qsort_best_span(a, pivot +1, high, cutoff);});
    }
}

// NOTE: No modifications are permitted / needed beyond this point

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Invalid command line - usage: <input file> <parallel cutoff>" << endl;
        exit(-1);
    }
    
    // Extract parameters
    ifstream ifile(argv[1]);
    int cutoff = atoi(argv[2]);
    
    // Set the number of threads
    task_scheduler_init init(8);
    
    // Read in the file
    int size;
    ifile >> size;
    int *orig = new int[size];
    int *a = new int[size];
    for (int i = 0; i < size; i++) {
        ifile >> orig[i];
    }
    
    
    // Run qsort_seq
    for (int i = 0; i < size; i++) a[i] = orig[i];
    tick_count start_time = tick_count::now();
    qsort_seq(a, 0, size - 1);
    tick_count end_time = tick_count::now();
    double seq_time = (end_time - start_time).seconds();
    cout << "qsort_seq results: " << endl;
    cout << "Time: " << seq_time << endl;
    cout << "Checking: ";
    check_array(a, size);
    cout << endl;
    
    //  Run p_qsort_inplace
    for (int i = 0; i < size; i++) a[i] = orig[i];
    start_time = tick_count::now();
    p_qsort_inplace(a, 0, size - 1, cutoff);
    end_time = tick_count::now();
    double p_inplace_time = (end_time - start_time).seconds();
    cout << "p_qsort_inplace results: " << endl;
    cout << "Time: " << p_inplace_time << endl;
    cout << "Checking: ";
    check_array(a, size);
    cout << endl;

    // Run p_qsort_best_span
    for (int i = 0; i < size; i++) a[i] = orig[i];
    start_time = tick_count::now();
    p_qsort_best_span(a, 0, size - 1, cutoff);
    end_time = tick_count::now();
    double p_best_span_time = (end_time - start_time).seconds();
    cout << "p_qsort_best_span results: " << endl;
    cout << "Time: " << p_best_span_time << endl;
    cout << "Checking: ";
    check_array(a, size);
    cout << endl;
    
    // Append the peformance results to the results file.
    ofstream ofile("qsort.csv", ios::app);
    ofile << size << "," << cutoff << ",";
    ofile << seq_time << ",",
    ofile << p_inplace_time << ",",
    ofile << p_best_span_time << endl;
    
    ofile.close();
    
    return 0;
}

// check_array: checks the contents of an array
void check_array(int *a, int size)
{
    for (int i = 0; i < size; i++) {
        if (a[i] != i + 1) {
            cout << "FAILED - first non-matching index is " << i << endl;
            print_array("contents", a, size);
            return;
        }
    }
    cout << "PASSED" << endl;
}

// print_array: prints the contents of an array
void print_array(string name, int *src, int size)
{
    cout << "Array " << name << ":" << endl;
    for (int i = 0; i < size; i++ ) {
        cout << src[i] << " ";
    }
    cout << endl;
}

