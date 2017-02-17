// CPSC 4600 / 5600 - Traveling Salesman Problem
//
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#include "tbb/tbb.h"
using namespace tbb;

// NOTE: It is not recommended that you modify the Graph class
bool found = false;
class Graph
{
public:
    Graph(int _size) {
        size = _size;
        array = new int[size * size];
    }
    
    ~Graph() { delete [] array; }
    
    int& operator()(int from, int to) {
        return array[from * size + to];
    }
    
    inline int getSize() const { return size; }
    
private:
    int size;
    int *array;
};

// TODO: This class is incomplete

class Path
{
public:
    int *pathArr;
    Path(int _size) {
        size = _size;
        pathArr = new int[size];
        
        for(int i = 0; i < size; i++){
            pathArr[i] = i;
        }
    }
    
    Path(const Path& p) {
        size = p.getSize();
        pathArr = new int [size];
        
        for(int i = 0; i < size; i++){
            pathArr[i] = p.pathArr[i];
        }
    }
    
    void swap(int a, int b){
        int temp = pathArr[a];
        pathArr[a] = pathArr[b];
        pathArr[b] = temp;
    }
    
    ~Path() {
        delete [] pathArr;
    }
    
    void print(){
        for(int i = 0; i < size; i++){
            cout<< pathArr[i] << " ";
        }
    }
    
    inline int getSize() const { return size;}
    
private:
    int size;
};

// TODO: Complete this function. You are allowed to modify the parameter list and/or return type of this function.

bool tsp(Graph &g, Path &p, int cost, int limit, int next)
{
    //use a curr variable
    int n = g.getSize(); //n is the size of the array
    if(cost > limit){
        return false;
    }else if(next == g.getSize() - 1){
        if((cost + g(p.pathArr[next], 0)) > limit){
            return false;
        }else{
            p.print();
            found = true;
            return true;
        }
        
    }else{
        int nextPlus = next + 1;
        
        for(int i = nextPlus; i < g.getSize(); i++){
            p.swap(nextPlus, i);
            int newCost = cost + g(p.pathArr[next], p.pathArr[nextPlus]);
            bool check = tsp(g, p, newCost, limit, nextPlus);
            if(check){
                return true;
            }
            p.swap(nextPlus, i);
        }
        return false;
    }
    
}



void parallel_tsp(Graph &g, Path &p, int cost, int limit, int next){
    parallel_for(1, g.getSize(), 1, [&](int val){
        Path paths(p);
        paths.swap(val, 1);
        int newCost = cost + g(paths.pathArr[0], paths.pathArr[1]);
        tsp(g, paths, newCost, limit, 1);
        
    });
}


// NOTE: Only modify the code segments with a TODO comment

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Invalid command line - usage: <input file> <limit>" << endl;
        exit(-1);
    }
    
    // Extract parameters
    ifstream ifile(argv[1]);
    int limit = atoi(argv[2]);
    
    // Get the size of the problem - on the first line of the input file.
    int size;
    ifile >> size;
    
    // Create and initialize data structures
    Graph g(size);
    
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            ifile >> g(row, col);
        }
    }
    
    // Initialize path
    // TODO: Modify to use appropriate constructor
    Path p(size);
    
    // Start the timer
    tick_count start_time = tick_count::now();
    
    // Execute the parallel algorithm
    // TODO: Modify if altered function signature.
    parallel_tsp(g, p, 0, limit, 0);
    //bool result = tsp(g, p, 0, limit, 0);
    
    // Stop the timer
    tick_count end_time = tick_count::now();
    double run_time = (end_time - start_time).seconds();
    cout<<endl;
    cout << "Time: " << run_time << endl;
    
    // Print out results
    // TODO: Modify to print out cost and path
    if (found) {
        cout << "Tour found!" << endl;
        
        // Print out cost and path
    }
    else {
        cout << "Tour not found" << endl;
    }
    
    return 0;
}
