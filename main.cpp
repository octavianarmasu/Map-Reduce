#include <pthread.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <set>
#include <cctype> 
#include "tema1.h"
using namespace std;

void* mapFunction(void* args) {
    MapperData* data = (MapperData*)args;

    for (const auto& file : data->files) {
        int fileIndex = (*data->fileToId)[file];
        ifstream infile(file);
        if (!infile.is_open()) {
            cout << "Error opening file: " << file << endl;
            continue;
        }
        string word;
        set<string> uniqueWords; 
        while (infile >> word) {
            string cleanedWord;
            // Remove non-alphabetic characters
            for (char ch : word) {
                if (isalpha(ch)) {
                    cleanedWord += tolower(ch);
                } else {
                  continue;
                }
            }
            word = cleanedWord;

            uniqueWords.insert(word);
        }
        infile.close();
        // Add data to the results vector (data->id was used for debugging)
        for (const auto& uniqueWord : uniqueWords) {
            data->results->push_back(make_tuple(uniqueWord, fileIndex, data->id));
        }
    }
    pthread_barrier_wait(data->barrier);
    return nullptr;
}

void* reducerFunction(void* args) {
    ReducerData* data = (ReducerData*)args;
    pthread_barrier_wait(data->barrier);

    map<string, set<int>> wordToFiles;

    // Populate the wordToFiles map with the data from the results vector from the mappers
    for (const auto& mapper : *data->mapperData) {
        for (const auto& result: *mapper.results) {
            const string& word = get<0>(result);
            if (data->targetLetters.find(word[0]) != data->targetLetters.end()) {
                int fileId = get<1>(result);
                wordToFiles[word].insert(fileId);
            }
        }
    }

    /*
        Create a file for each target letter and write the words and the files 
        they appear in or empty files if no word starts with the target letter
     */
    for (char letter : data->targetLetters) {
        string filename(1, letter);
        filename += ".txt";
        ofstream outfile(filename);
        if (!outfile.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            continue;
        }

        vector<pair<string, set<int>>> wordList;
        for (const auto& [word, fileSet] : wordToFiles) {
            if (word[0] == letter) {
                wordList.emplace_back(word, fileSet);
            }
        }

        sort(wordList.begin(), wordList.end(), compareWordList);    

        for (const auto& [word, fileSet] : wordList) {
            outfile << word << ":[";
            for (auto it = fileSet.begin(); it != fileSet.end(); ++it) {
                if (it != fileSet.begin())
                    outfile << " ";
                outfile << *it;
            }
            outfile << "]\n";
        }
        outfile.close();
    }
    return nullptr;
}

int main(int argc, char** argv) {
    int numMappers = stoi(argv[1]);
    int numReducers = stoi(argv[2]);
    string file = argv[3];

    ifstream infile(file);
    if (!infile.is_open()) {
        cout << "Error opening file: " << file << endl;
        return 1;
    }

    int numFiles;
    infile >> numFiles;
    vector<string> files(numFiles);
    for (int i = 0; i < numFiles; i++) {
      infile >> files[i];
    }
    infile.close();

    map<string, int> fileToId;
    for (int i = 0; i < numFiles; i++) {
        fileToId[files[i]] = i + 1;
    }

    // Initialize barrier to synchronize threads
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, nullptr, numMappers + numReducers);

    // Initialize mapper data and assign files to mappers
    vector<MapperData> mapperData(numMappers);
    for (int i = 0; i < numMappers; i++) {
        mapperData[i].id = i;
        mapperData[i].results = new vector<tuple<string, int, int>>();
        mapperData[i].fileToId = &fileToId;
        mapperData[i].barrier = &barrier;
    }

    for (int i = 0; i < numFiles; i++) {
        mapperData[i % numMappers].files.push_back(files[i]);
    }

    // Initialize reducer data and assign target letters to reducers
    vector<ReducerData> reducerData(numReducers);
    for (int i = 0; i < numReducers; i++) {
        set<char> assignedLetters;
        for (char letter = 'a' + i; letter <= 'z'; letter += numReducers) {
            assignedLetters.insert(letter);
        }
        reducerData[i].targetLetters = assignedLetters;
        reducerData[i].mapperData = &mapperData;
        reducerData[i].barrier = &barrier;
    }

    vector<pthread_t> threads(numMappers);
    vector<pthread_t> reducerThreads(numReducers);

    for (int i = 0; i < numMappers + numReducers; i++) {
        if (i < numMappers) {
            pthread_create(&threads[i], nullptr, mapFunction, &mapperData[i]);
        } else {
            pthread_create(&reducerThreads[i - numMappers], nullptr, reducerFunction, &reducerData[i - numMappers]);
        }
    }

    for (int i = 0; i < numMappers + numReducers; i++) {
        if (i < numMappers) {
            pthread_join(threads[i], nullptr);
        } else {
            pthread_join(reducerThreads[i - numMappers], nullptr);
        }
    }

    /*
        Deallocate memory
    */
    for (int i = 0; i < numMappers; i++) {
        delete mapperData[i].results;
    }
    pthread_barrier_destroy(&barrier);

    return 0;
}