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

long sum = 0;
int jump = 0;

void *runner(void *upper);

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Usage: sum_serial.out <integer value>\n");
        return -1;
    }

    int numThreads = atoi(argv[1]), n = atoi(argv[2]);
    // int numThreads = 3, n = 10;
    if(n <= 0 || numThreads <= 0) {
        if(n <= 0) fprintf(stderr, "%d must be > 0\n", n);
        if(numThreads <= 0) fprintf(stderr, "%d must be > 0\n", numThreads);
        return -1;
    }

    jump = (int)(n / numThreads);
    pthread_t tid[numThreads];
    pthread_attr_t attr[numThreads];
    int upper;
    for(upper = jump; upper <= n; upper += jump) {
        /* get the default attributes */
        int i = upper / jump - 1;
        pthread_attr_init(&attr[i]);
        /* create the thread */
        pthread_create(&tid[i], &attr[i], runner, &upper);        
        /* wait for the thread to exit */
        pthread_join(tid[i], NULL);
    }

    if(upper > n) for(upper = upper - jump + 1; upper <= n; sum += upper, upper ++);

    printf("%ld\n", sum);

    return 0;   
}

void *runner(void *param) {
    int upper = *(int*)param;
    for(int i = upper - jump + 1; i <= upper; sum += i, i++);
    pthread_exit(NULL);
}