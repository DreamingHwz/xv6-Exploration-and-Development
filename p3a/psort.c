#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

#define RECORD_LEN 100

typedef struct {
    int key;
    char value[96];
} record_t;
typedef record_t* record_ptr;

typedef struct {
    int curr; 
    int num;
    int length;
} threads_t;

record_ptr* records = NULL;
int filesize;
int num_records;
int procs;
int inputfd;

pthread_mutex_t lock;

int cmp(const void *a,const void *b)
{
    record_ptr *aa = (record_ptr *) a;
    record_ptr *bb = (record_ptr *) b;
    return ((*aa)->key > (*bb)->key);
}

void error(){
    if (records != NULL) 
        free(records);
    char error_message[30] = "An error has occurred\n";
    if (write(STDERR_FILENO, error_message, strlen(error_message)) != 0)
        exit(0);
}


record_ptr inputMmap(char* name){
    struct stat statbuf;
    record_ptr mapped;

    if ((inputfd = open(name, O_RDONLY, 0600)) == -1)
        error();
    stat(name, &statbuf);
    filesize = statbuf.st_size;
    if ((mapped = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, inputfd, 0)) == MAP_FAILED)
        error();

    //close(inputfd);
    return mapped;
}


record_ptr* initRecords(record_ptr mapped){
    num_records = filesize / RECORD_LEN;
    record_ptr* arr_records = malloc(num_records * sizeof(record_ptr));

    for (int i = 0; i < num_records; i++)
        arr_records[i] = mapped + i;

    return arr_records;
}


void output(record_ptr* records, char* filename){
    int fp;
    record_ptr output;
    
    if ((fp = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600)) == -1)
        error();
    if (ftruncate(fp, filesize) == -1)
        error();
    if ((output = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0)) == MAP_FAILED)
        error();
    for (int i = 0; i < num_records; i++)
        output[i] = *records[i];
    
    fsync(fp);
    munmap(output, filesize);
    close(fp);
}

void merge(int left, int mid, int right) {
    int curr_left = mid - left + 1;
    int curr_right = right - mid;
    record_t **L = malloc(curr_left * sizeof(record_t *));
    record_t **R = malloc(curr_right * sizeof(record_t *));
    for (int i = 0; i < curr_left; i++)
        L[i] = records[left + i];
    for (int j = 0; j < curr_right; j++)
        R[j] = records[mid + 1 + j];
    int i = 0;
    int j = 0;
    int k = left;
    while (i < curr_left && j < curr_right) {
        if (L[i]->key <= R[j]->key) 
            records[k] = L[i++];
        else 
            records[k] = R[j++];
        k++;
    }
    while (i < curr_left) {
        records[k++] = L[i++];
    }
    while (j < curr_right) {
        records[k++] = R[j++];
    }
    free(L);
    free(R);

    // int i = 0, j = 0, k = left;
    // record_ptr *leftarr = malloc((mid - left + 1) * sizeof(record_t *));
    // record_ptr *rightarr = malloc((right - mid) * sizeof(record_t *));
    
    // for (int t = 0; t < mid - left + 1; t++)
    //     leftarr[t] = records[left + t];
    // for (int t = 0; t < right - mid; t++)
    //     rightarr[t] = records[mid + 1 + t];

    // while (i < mid - left + 1 && j < right - mid) {
    //     if (leftarr[i]->key <= rightarr[j]->key)
    //         records[k++] = leftarr[i++];
    //     else records[k++] = rightarr[j++];
    // }

    // while (i < mid - left + 1) 
    //     records[k++] = leftarr[i++];
    // while (j < right - mid)
    //     records[k++] = rightarr[j++];
    
    // free(leftarr);
    // free(rightarr);
}

void mergeSort(int left, int right){
    if (left >= right) return;
    int mid = left + (right - left) / 2;

    mergeSort(left, mid);
    mergeSort(mid + 1, right);
    merge(left, mid, right);
}


void* threads_mergeSort(void *threads) {
    // pthread_mutex_lock(&lock);
    // int nprocs = get_nprocs();

    // if (num_records < 100 || nprocs >= num_records)
    //     nprocs = 1;

    // int curr_thread = (long)threads;
    // int size = num_records / nprocs;
    // int lastsize = num_records % nprocs;

    // int left = curr_thread * size;
    // int right = left + size;
    // if (curr_thread == nprocs - 1){
    //     right += lastsize;
    // }
    // int mid = left + (right - left) / 2;

    // if (right > left + 1) {
    //     mergeSort(left, mid);
    //     mergeSort(mid, right);
    //     merge(left, mid, right);
    // }
    // pthread_mutex_unlock(&lock);
    // return ((void *)0);
    int curr_thread = (long)threads;
    int left = curr_thread * (num_records / procs);
    int right = (curr_thread == procs - 1) ? (num_records - 1) : (left + num_records / procs - 1);
    mergeSort(left, right);
    return 0;
}

void* threads_merge(void * threads) {
    threads_t *thread = (threads_t *) threads;

    int left = thread->curr * thread->num;
    int mid = left + thread->num / 2 - 1;
    int right = left + thread->num - 1;

    int last_index = thread->length / 2 - 1;
    int last = (thread->length % 2 != 0 || thread->curr != last_index) ? 0 : 1;
    if (last == 1) right = num_records - 1;
    
    merge(left, mid, right);

    return 0;
}


void multithread_sort(){
    procs = get_nprocs();
    if (num_records <= 100 || procs >= num_records)
        procs = 1;
    pthread_t threads[procs];
    pthread_t curr_thread;

    //create threads;
    curr_thread = 0;
    while (curr_thread < procs) {
        pthread_create(&threads[curr_thread], NULL, threads_mergeSort, (void *)curr_thread);
        curr_thread++;
    }

    //waiting for the threads
    curr_thread = 0;
    while (curr_thread < procs) {
        pthread_join(threads[curr_thread], NULL);
        curr_thread++;
    }

    //merge the records sorted by each thread
    if (procs > 1) {
        int length = procs;
        int num = num_records / length * 2;

        int merges;
        for (merges = length / 2; merges > 0; merges = length / 2) {
            pthread_t threads[merges];
            threads_t *curr_thread = malloc(merges * sizeof(threads_t));

            int temp = length;
            int i = 0;
            while (i < merges) {
                curr_thread[i].curr = i;
                curr_thread[i].num = num;
                curr_thread[i].length = length;
                pthread_create(&threads[i], NULL, threads_merge, (void *)&curr_thread[i]);
                i++;
                temp--;
            }
            
            i = 0;
            while (i < merges) {
                pthread_join(threads[i], NULL);
                i++;
            }

            free(curr_thread);
            num *= 2;
            length = temp;
        }
    }

    //mergeSort(records, 0, num_records);
}


int main(int argc, char* argv[]){
    if (argc < 3) 
        error();
    record_ptr input = inputMmap(argv[1]); //argv[1]
    records = initRecords(input);

    //qsort(records, num_records, sizeof(record_ptr), cmp);
    //mergeSort(0, num_records);
    multithread_sort();

    output(records, argv[2]);
    
    munmap(input, filesize);
    free(records);
    close(inputfd);
}
