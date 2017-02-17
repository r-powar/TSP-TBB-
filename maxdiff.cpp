// CPSC 4600 / 5600 - Maximum Difference
//
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "tbb/tbb.h"
using namespace tbb;

int getMaxVal(int leftdiff, int rightdiff, int crossdiff){
    if(leftdiff > crossdiff && leftdiff > rightdiff){
        return leftdiff;
    }else if (rightdiff > crossdiff && rightdiff > leftdiff){
        return rightdiff;
    }else{
        return crossdiff;
    }
}

int calculateMin(int *src, int low, int high){
    int min;
    min = src[low];
    for(int i = low; i <= high; i++){
        if(src[i] < min){
            min = src[i];
        }
    }
    return min;
}

int calculateMax(int *src, int low, int high){
    int max;
    max = src[low];
    
    for(int i = low; i <= high; i++){
        if(src[i] > max){
            max = src[i];
        }
    }
    return max;
}



int calculateMaxDiff(int *src, int start, int end){
    
    if(start >= end){
        return 0;
    }
    int middle = (start + end)/ 2;
    int half1;
    int half2;
    int leftMin;
    int rightMax;
    int cross;
    parallel_invoke([&]{half1 = calculateMaxDiff(src, start, middle);},
                    [&]{half2 = calculateMaxDiff(src, middle + 1, end);});
    leftMin = calculateMin(src, start, middle);
    rightMax = calculateMax(src, middle + 1, end);
    cross = rightMax - leftMin;
    
    return getMaxVal(half1, half2, cross);
};


int compute_max_diff(int *src, int size)
{
    int maxDiff = calculateMaxDiff(src, 0, size - 1);
    
    return maxDiff;
    
}

// NOTE: No modifications are permitted / needed beyond this point

int
main(int argc, char *argv[]){

    if (argc != 2) {
        cerr << "Invalid command line - usage: <input file>" << endl;
        exit(-1);
    }
    
    // Extract parameters
    ifstream ifile(argv[1]);
    
    // Read in the file
    int size;
    ifile >> size;
    int *src = new int[size];
    for (int i = 0; i < size; i++) {
        ifile >> src[i];
    }


    // Run algorithm
    int max_diff = compute_max_diff(src, size);
    cout << max_diff << endl;
    
    return 0;
}
