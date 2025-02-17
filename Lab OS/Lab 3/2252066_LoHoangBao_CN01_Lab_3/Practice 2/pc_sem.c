#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_ITEMS 1
#define THREADS 1 // 1 producer and 1 consumer
#define LOOPS 6 // variable

// Initiate shared buffer
int buffer[MAX_ITEMS];
int fill = 0;
int use = 0;
sem_t semaphore_read;
sem_t semaphore_write;

void put(int value); // put data into buffer
int get(); // get data from buffer

void *producer(void *arg) {    
    int i;
    int tid = *(int*)arg;
    for(i = 0; i < LOOPS; i++) {
        sem_wait(&semaphore_write);
        put(i); // line P2
        printf("Producer %d put data %d\n", tid, i);
        sleep(1);
        sem_post(&semaphore_read);
    }
    
    pthread_exit(NULL);    
}

void *consumer(void *arg) {    
    int i, tmp = 0;
    int tid = *(int*)arg;
    while(tmp != -1) {     
        sem_wait(&semaphore_read);
        tmp = get(); // line C2
        printf("Consumer %d get data %d\n", tid, tmp);
        sleep(1);
        sem_post(&semaphore_write);
    }

    pthread_exit(NULL);    
}

int main(int argc, char **argv) {
    int i, j;
    int tid[THREADS];
    pthread_t producers[THREADS];
    pthread_t consumers[THREADS];  

    sem_init(&semaphore_write, 0, 1);  
    sem_init(&semaphore_read, 0, 0);

    for(i = 0; i < THREADS; i ++) {
        tid[i] = i;
        // Create producer thread
        pthread_create(&producers[i], NULL, &producer, &tid[i]);

        // Create consumer thread
        pthread_create(&consumers[i], NULL, &consumer, &tid[i]);
    }

    for(i = 0; i < THREADS; i ++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&semaphore_write);
    sem_destroy(&semaphore_read);

    return 0;
}

void put(int value) {
    buffer[fill] = value; // line f1
    fill = (fill + 1) % MAX_ITEMS; // line f2    
}

int get() {
    int tmp = buffer[use]; // line g1
    use = (use + 1) % MAX_ITEMS; // line g2
    return tmp;
}