// PPLS Exercise 2 Starter File
//
// See the exercise sheet for details
//
// Note that NITEMS, NTHREADS and SHOWDATA should
// be defined at compile time with -D options to gcc.
// They are the array length to use, number of threads to use
// and whether or not to printout array contents (which is
// useful for debugging, but not a good idea for large arrays).

/* *** DICUSSION ***
 * Because we have been told not to change the main function, I decided to do a
 * sanity check of the compile time arguments in the parallelprefixsum function,
 * just to be sure. For instance, it makes no sense not to have any items in the
 * array, or to have no threads (but if given one thread the parallelprefixsum
 * function will still work, albeit in a less efficient way than simply using
 * sequentialprefixsum). Finally, I check that there are more items in the array
 * than there are threads, as required in the specification.
 *
 * I have decided to allocate the number of elements in each thread in a straight
 * forward way: use the (implicit) floor of the division between the number of
 * items and the number of threads, and allocate the remainder to the last one
 * For instance: if given 10 items and 4 threads, the first three will have 2 and
 * the last one will have 4 items since floor(10/4) = 2 and remainder(10/4) = 2.
 *
 * After this, I use a fork join method of creating the threads in the parallelprefixsum
 * function, while the main logic of the algorithm is in *threadedsum. This is
 * further divided into three phases, as described in the specification.
 *
 * Probably the most important aspect of my implementation is the use of the two
 * barriers, after phase one and phase two are complete. It's a custom DIY barrier
 * as described in the lecture, since I did not manage to use the (optional) POSIX
 * pthread_barrier_t either on my Mac or the DICE machines. I could have created
 * separate header and source files to create and use this type, but the mutex
 * together with the conditional variable work well to ensure that all threads
 * complete each phase before moving on to the next one.
 *
 * The reason why this is important is that we can't be sure in what order the threads
 * will execute and interleave, and Thread 0 has to edit the already calculated
 * partial sums, otherwise they might overwrite its Phase 2 calculations
 *
 * Furthermore, thread 0 has to finish its prefixing of top elements, because
 * the other threads rely on these being correct in order to update their own
 * remaining elements. Thus, we need 2 barriers at which to stop and synch the threads
 * ensuring consistency between their results.
 *
 * All of the calculations are done in place, since they use pointers, and there is
 * no copying of the arrays.
 *
 * Sidenote: I probably enjoyed these courseworks the most this year. Thanks!
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int num_arrived = 0;
pthread_mutex_t barrier;
pthread_cond_t go;

typedef struct arg_pack {
    int *arg_data;
    int arg_num;
    int arg_id;
} arg_pack;

// Print a helpful message followed by the contents of an array
// Controlled by the value of SHOWDATA, which should be defined
// at compile time. Useful for debugging.
void showdata (char *message,  int *data,  int n) {
    int i;

    if (SHOWDATA) {
        printf (message);
        for (i=0; i<n; i++ ){
            printf (" %d", data[i]);
        }
        printf("\n");
    }
}

// Check that the contents of two integer arrays of the same length are equal
// and return a C-style boolean
int checkresult (int *correctresult,  int *data,  int n) {
    int i;

    for (i=0; i<n; i++ ){
        if (data[i] != correctresult[i]) return 0;
    }
    return 1;
}

// Compute the prefix sum of an array **in place** sequentially
void sequentialprefixsum (int *data, int n) {
    int i;
    for (i=1; i<n; i++ ) {
        data[i] = data[i] + data[i-1];
    }
}

void Barrier() {
    // Custom barrier using mutex locks and conditional variables declared globally
    pthread_mutex_lock(&barrier);
    num_arrived += 1;

    if (num_arrived == NTHREADS) {
        num_arrived = 0;
        pthread_cond_broadcast(&go);
    } else {
        pthread_cond_wait(&go, &barrier);
    }

    pthread_mutex_unlock(&barrier);
}

void *threadedsum(void *args) {
    int i;
    arg_pack *thread_args = (arg_pack *) args;

    // NITEMS and NTHREADS could have been packed into the arg_pack,
    // but since they are available globally anyway, I didn't bother, plus it saves space
    int start_pos = (NITEMS / NTHREADS) * thread_args->arg_id;
    int final_pos = start_pos + thread_args->arg_num - 1;

    // PHASE 1:
    for (i=start_pos+1; i<start_pos+thread_args->arg_num; i++) {
        thread_args->arg_data[i] += thread_args->arg_data[i-1];
    }
    Barrier();

    // PHASE 2:
    if (thread_args->arg_id == 0) {
        int current_elem_pos = thread_args->arg_num - 1;
        int final_elem_pos = NITEMS - 1;

        // In case NTHREADS == 1, it skips this contional completely,
        // and continues to Phase 3, in which it does nothing, since it's
        // thread_args->arg_id is equal to 0.
        // The sum is already correct by phase 1!
        int max_elem = thread_args->arg_data[current_elem_pos];
        if (NTHREADS == 2) {
            thread_args->arg_data[final_elem_pos] += max_elem;
        }
        else if (NTHREADS > 2) {
            current_elem_pos += thread_args->arg_num;
            for (i=1; i<NTHREADS-1; i++) {
                thread_args->arg_data[current_elem_pos] += max_elem;
                max_elem = thread_args->arg_data[current_elem_pos];
                current_elem_pos += thread_args->arg_num;
            }
            thread_args->arg_data[final_elem_pos] += max_elem;
        }
    }
    Barrier();

    // PHASE 3:
    if (thread_args->arg_id != 0) {
        int update_val = thread_args->arg_data[start_pos-1];
        for (i=start_pos; i<final_pos; i++) {
            thread_args->arg_data[i] += update_val;
        }
    }
}

// YOU MUST WRITE THIS FUNCTION AND ANY ADDITIONAL FUNCTIONS YOU NEED
void parallelprefixsum (int *data, int n) {
    /* PHASE 0: Create the threads, after doing the above checks
     * PHASE 1: Every thread performs a prefix sum sequentially across its chunk of data, in place.
     * PHASE 2: Thread 0 performs a sequential prefix sum using just the highest indexed position from
each chunk, in place. Other threads simply wait.
     * PHASE 3: Every thread (except thread 0) adds the final value from the preceding chunk into every
value in its own chunk, except the last position (which already has its correct value after phase 2), in place.
     */

    int i;

    // There has to be at least one thread, otherwise what's the point? :)
    if (NTHREADS < 1) {
        printf("Please specify at least 1 thread! .... exiting\n");
        exit(EXIT_FAILURE);
    }

    // There has to be at least one element in the array
    if (n < 1) {
        printf("Please add more items to the array! .... exiting\n");
        exit(EXIT_FAILURE);
    }

    // First check that there are more items in the array than there are threads
    // If not, exit the program
    if (n < NTHREADS) {
        printf("Please add more items to the array than there are threads! .... exiting\n");
        exit(EXIT_FAILURE);
    }

    pthread_t *threads;
    arg_pack  *threadargs;

    threads     = (pthread_t *) malloc (NTHREADS * sizeof(pthread_t));
    threadargs  = (arg_pack *)  malloc (NTHREADS * sizeof(arg_pack));

    // Have to use mutex instead of barriers because pthread_barrier_t is not supported on macOS, on which I am developing
    // Tested on DICE, works as it should, but pthread_barrier_t still is not supported for some reason
    pthread_mutex_init(&barrier, NULL);
    pthread_cond_init(&go, NULL);

    // Calculate the number of elements each thread will receive
    // Each thread will only work on its allocated elements (except thread 0)
    // Based on its arg_id and number of elements
    for (i=0; i<NTHREADS; i++) {
        threadargs[i].arg_id = i;
        threadargs[i].arg_data = data;
        if (i != NTHREADS - 1)
            threadargs[i].arg_num = n / NTHREADS;
        else
            // Allocate the remainder to the last thread
            threadargs[i].arg_num = n / NTHREADS + n % NTHREADS;

        // Create the threads in the same for-loop in which the arg_pack
        // is created, to save some computational time, since they will have to
        // wait at the first barrier anyway
        pthread_create(&threads[i], NULL, threadedsum, (void *) &threadargs[i]);
    }

    for (i=0; i<NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}


int main (int argc, char* argv[]) {

    int *arr1, *arr2, i;

    // Check that the compile time constants are sensible for this exercise
    if ((NITEMS>10000000) || (NTHREADS>32)) {
        printf ("So much data or so many threads may not be a good idea! .... exiting\n");
        exit(EXIT_FAILURE);
    }

    // Create two copies of some random data
    arr1 = (int *) malloc(NITEMS*sizeof(int));
    arr2 = (int *) malloc(NITEMS*sizeof(int));
    srand((int)time(NULL));
    for (i=0; i<NITEMS; i++) {
        arr1[i] = arr2[i] = rand()%5;
    }
    showdata ("initial data          : ", arr1, NITEMS);

    // Calculate prefix sum sequentially, to check against later
    sequentialprefixsum (arr1, NITEMS);
    showdata ("sequential prefix sum : ", arr1, NITEMS);

    // Calculate prefix sum in parallel on the other copy of the original data
    parallelprefixsum (arr2, NITEMS);
    showdata ("parallel prefix sum   : ", arr2, NITEMS);

    // Check that the sequential and parallel results match
    if (checkresult(arr1, arr2, NITEMS))  {
        printf("Well done, the sequential and parallel prefix sum arrays match.\n");
    } else {
        printf("Error: The sequential and parallel prefix sum arrays don't match.\n");
    }

    return 0;
}
