#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

int jump = 0;

/* struct to save the data after runner function execute */
typedef struct thread_data {
    int upper;
    long long local_sum;
} thread_data;

/* function to calculate sum for each thread */
void *runner(void *upper);

int main(int argc, char *argv[]) {
    if(argc != 3) { /* check if the number of argument is not 3 */
        fprintf(stderr, "Usage: sum_serial.out <integer value>\n");
        return -1;
    }

    int numThreads = atoi(argv[1]);
    long long n = atoll(argv[2]);
    if(n <= 0 || numThreads <= 0) { /* check if n is negative or numThreads is negative */
        if(n <= 0) fprintf(stderr, "%lld must be > 0\n", n); /* check if n is negative */
        if(numThreads <= 0) fprintf(stderr, "%d must be > 0\n", numThreads); /* check if numThreads is negative */
        return -1;
    }

    /* calculate the segment for calculate sum for each thread and assign for jump */
    jump = (int)(n / numThreads);
    pthread_t tid[numThreads];
    pthread_attr_t attr[numThreads];
    thread_data data[numThreads];
    int upper; long long sum = 0;
    for(upper = jump; upper <= n; upper += jump) {
        /* get the default attributes */
        int i = upper / jump - 1;
        data[i].upper = upper;
        pthread_attr_init(&attr[i]);
        /* create the thread */
        pthread_create(&tid[i], &attr[i], runner, (void*)&data[i]);
    }

    for(int i = 0; i < numThreads; i++) {
        /* wait for the thread to exit */
        pthread_join(tid[i], NULL);
        sum += data[i].local_sum;
    }

    /* if the last segment is not calculate then the main thread will do it */
    if(upper > n) for(upper = upper - jump + 1; upper <= n; sum += upper, upper ++);

    if(n == 1) printf("sum(1) = 1\n");
    else if(n == 2) printf("sum(2) = 1 + 2 = 3\n");
    else printf("sum(%lld) = 1 + 2 + .... + %lld = %lld\n", n, n, sum);

    return 0;   
}

/* function to calculate sum for each thread */
void *runner(void *param) {
    thread_data *tdata = (thread_data*)param;
    int upper = tdata->upper;
    long long local_sum = 0;
    for(int i = upper - jump + 1; i <= upper; local_sum += i, i++);
    tdata->local_sum = local_sum;
    pthread_exit(NULL);
}