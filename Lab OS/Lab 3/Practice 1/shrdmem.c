#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int MAX_COUNT = 1e9;
static int count = 0;
pthread_mutex_t lock;

void *f_count(void *sid) {
    // pthread_mutex_lock(&lock);
    int i;
    for(i = 0; i < MAX_COUNT; i++) {
        pthread_mutex_lock(&lock);
        count = count + 1;
        pthread_mutex_unlock(&lock);
    }

    printf("Thread %s: holding %d\n", (char*)sid, count);
    // pthread_mutex_unlock(&lock);
}

int main() {
    pthread_t thread1, thread2;

    /* Create independent threads each of which will execute function */
    pthread_create(&thread1, NULL, &f_count, "1");
    pthread_create(&thread2, NULL, &f_count, "2");

    // Wait for thread th1 finish
    pthread_join(thread1, NULL);

    // Wait for thread th2 finish
    pthread_join(thread2, NULL);

    // Destroy the mutex object
    pthread_mutex_destroy(&lock);
    
    return 0;
}