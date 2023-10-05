/* merge sort using nested threads  */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int* global;


#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void merge_sort(void *array, void *size);

int main(int argc, char *argv[])
{
   ppid = getpid();

   /*
   1. Create global array and populate it
   2. invoke merge sort (array ptr, size)

   Merge sort:
   0. base case - size = 1 --> return 
   1. thread create with merge sort (array left, size/2)
   2. thread create with merge sort (array + size/2, size - size/2)
   3. join both threads
   4. Merge function
   */


    int size = 11;
    global = (int*)malloc(size * sizeof(int));
    for(int i = 0; i < size; i++){
        global[i] = size - i - 1;
    }

   int thread_pid = thread_create(merge_sort, global, &size);
   assert(thread_pid > 0);

   int join_pid = thread_join();
   assert(join_pid == thread_pid);
   assert(global[0] == 0);
   assert(global[10] == 10); 

   printf(1, "TEST PASSED\n");
   exit();
}

void merge(int* array, int* array_right,int size_left, int size_right,int*temp_array){
    int i = 0;
    int j = 0;
    int k = 0;
    while(i < size_left && j < size_right){
        if(array[i] < array_right[j]){
            temp_array[k] = array[i];
            i++;
        }
        else{
            temp_array[k] = array_right[j];
            j++;
        }
        k++;
    }
    while(i < size_left){
        temp_array[k] = array[i];
        i++;
        k++;
    }
    while(j < size_right){
        temp_array[k] = array_right[j];
        j++;
        k++;
    }
    for(int i = 0; i < size_left + size_right; i++){
        array[i] = temp_array[i];
    }
   
}

void merge_sort(void *arg1, void *arg2) {
   int *array = (int*)arg1;
   int size = *(int*)arg2;

   if (size==1){
       exit();
   }

   
   int size_left = size/2;
   int size_right = size-size/2;

   int* array_right = (int*)(array + size_left);
//    merge_sort(array_right, size_right);

//    int nested_thread_pid_l = thread_create(merge_sort, array, &size_left);
//    int nested_thread_pid_r = thread_create(merge_sort, array_right, &size_right);
   
   thread_create(merge_sort, array, &size_left);
   thread_create(merge_sort, array_right, &size_right);

   thread_join();
   thread_join();
 

//    int nested_join_pid_1 = thread_join();
//    int nested_join_pid_2 = thread_join();

   int* temp_array = malloc(size*sizeof(int));

   merge(array,array_right,size_left,size_right,temp_array);

   free(temp_array);

//    assert(nested_thread_pid_l == nested_join_pid_1 || nested_thread_pid_l == nested_join_pid_2);
//    assert(nested_thread_pid_r == nested_join_pid_1 || nested_thread_pid_r == nested_join_pid_2);
   exit();
}

