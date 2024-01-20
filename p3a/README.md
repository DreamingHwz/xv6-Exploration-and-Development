# Parallel Sort
It is a multi-thread merge sort.

## Introduction
```
typedef struct {
    int key;
    char value[96];
} record_t;
typedef record_t* record_ptr;
```
* ```record_ptr inputMmap(char* name)```: to map the input file into the address space, then access bytes of the input file via pointers.
* ```record_ptr* initRecords(record_ptr mapped)```: init an array of ptr to each records.
* ```void multithread_sort()```: do multi-thread sort. Each thread sorts part of the records array.
* ```void output(record_ptr* records, char* filename)```: Output the records array to file.

## Usage

```
gcc -Wall -Werror -pthread -O psort.c -o psort
./psort input output
```


## Author

Skylar Hou, Yuxin Liu