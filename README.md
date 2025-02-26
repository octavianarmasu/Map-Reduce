#### Copyright Armasu Octavian

## Map-Reduce 

### Problem statement

Implemented in C++ a simple map-reduce algorithm that calculates an inverted
index for a set of documents. The index contains the list of all the words from
the input documents with the list of documents where they appear. The program
uses a parallel implementation of the map-reduce algorithm, using PThreads.

### Implementation details

The program receives as input the number of threads for the map phase, the number
of threads for the reduce phase, a `.txt` file. The file contains on the first line
the number of documents that are in the file, and on the following lines the names
of the documents.

#### How to run
In the directory, run the following commands:
``````
make
./tema1 <number_of_threads_map> <number_of_threads_reduce> <input_file>
``````
After running the program make sure to remove the executable file by running:
``````
make clean
``````

### Map logic
To store the data necessary for the map function, I used a struct `MapperData` as follows:
``````
struct MapperData {
    vector<string> files; // files to process
    int id; // id of the mapper thread
    vector<tuple<string, int, int>>* results; // word, file id, mapper id 
    map<string, int>* fileToId; // map file name to file id
    pthread_barrier_t* barrier; // barrier to synchronize threads
}; 
``````
After reading the input data, store the data in a vector of `MapperData`.
The allocation of threads is done statically, each thread processes a file
using the formula: `i % number_of_threads_map`, where i is the current file id.
So, for example, if there were to be 3 threads and 5 files, the first thread
would process files 0, 3, the second thread 1, 4, and the third thread 2.

#### Map function

First, in the map function I iterate through the files that were assigned to the
current thread. For each file, I read word by word the file. After reading a word,
I make sure it doesn't contain any special characters such as `.,!?-` and has all
the letters lowercase. If the word is valid, I add it to a set of string `uniqueWords`.
After reading all the data from a file, iterate through the set of unique words and
push the tuple `(word, file id, mapper id)` to the results vector.

Before the return at the end of the function, I use a barrier to synchronize the threads.
The length of the barrier is the number of threads for the map phase plus the number of
threads for the reduce phase.

### Reduce logic
To store the data necessary for the reduce function, I used a struct `ReducerData` as follows:
``````
struct ReducerData {
    set<char> targetLetters; // letters to process
    vector<MapperData>* mapperData; // data from mappers
    pthread_barrier_t* barrier; // barrier to synchronize threads
};
``````
Again the allocation of threads is done statically, each thread receives a set of letters
to process. For example if there were to be 3 reducer threads and use all the letters from
the alphabet, the first thread would process the letters `a-i`, the second thread `j-r` and
the third thread `s-z`.

#### Reduce function

First, I use a barrier to synchronize the threads. After that, I iterate through the
data from the mappers. For each mapper, I iterate through the results vector and 
check to see if the letter the word starts with is in the set of letters that the
current reducer thread has to process. If it is, I add the word to a map 
`map<string, set<int>> wordToFiles`. The map contains the word and the set of files
where the word appears. 

After, I iterate through the assigned letters and create, if it doesn't exist, a file
with the name `<letter>.txt`. I use a vector of paris `vector<pair<string, set<int>>>`
to store the data from the map. After that, I sort the vector using the 
`compareWordList` function that sorts the words alphabetically, if the number of 
files is the same, or by the number of files in descending order.

### Parallelism

I use only one for the start the threads for the mapper and reducer. To make sure
that they start properly, I use a barrier to synchronize the threads. The length of 
the barrier is the number of threads for the map phase plus the number of threads
for the reduce phase.

### Conclusion

In the end of the program, I deallocate the memory used for the barrier and the 
results vector in mapperData.
