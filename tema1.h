#include <pthread.h>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <set>
#include <cctype> 
using namespace std;

// struct to pass data to mapper threads
struct MapperData {
    vector<string> files; // files to process
    int id; // id of the mapper thread
    vector<tuple<string, int, int>>* results; // word, file id, mapper id (optional, used for  debugging)
    map<string, int>* fileToId; // map file name to file id
    pthread_barrier_t* barrier; // barrier to synchronize threads
}; 

// struct to pass data to reducer threads
struct ReducerData {
    set<char> targetLetters; // letters to process
    vector<MapperData>* mapperData; // data from mappers
    pthread_barrier_t* barrier; // barrier to synchronize threads
};

/*
    Compare function to sort word list by 2 criteria:
        1. Alphabetically if the number of files is the same
        2. By number of files in descending order
 */
bool compareWordList (const pair<string, set<int>>& a, const pair<string, set<int>>& b) {
    if (a.second.size() == b.second.size())
        return a.first < b.first; // sort alphabetically
    return a.second.size() > b.second.size(); // sort by number of files
}
void* mapFunction(void* args);
void* reducerFunction(void* args);