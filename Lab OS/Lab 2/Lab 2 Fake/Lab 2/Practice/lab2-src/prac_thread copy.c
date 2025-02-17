#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;
void* runner(void* param) {
    int i, n;
    n = atoi(param);
    printf("Thread called: \n");
    printf("n = %d , ", n);
    sum = 0;
    for (i=1; i<=n; i++) {
        sum += i;
    }    
    pthread_exit(0);
}

int main(int agrc, char** agrv) {
    pthread_t tid;
    char* param = "10";

    pthread_create(&tid, NULL, runner, param);
    
    pthread_join(tid, NULL);
    printf("Variable sum is: %d \n", sum);
    return 0;
}