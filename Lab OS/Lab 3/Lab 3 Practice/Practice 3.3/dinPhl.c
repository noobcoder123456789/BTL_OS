#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5

pthread_mutex_t mtx;
pthread_cond_t chopstick[N];
enum {THINKING, HUNGRY, EATING} state[N];

// Function Declaration
void *philosopher(void*);
void eat(int);
void think(int);

int main() {
    int i, a[N];
    pthread_t tid[N];

    // Initialize the mutex lock
    pthread_mutex_init(&mtx, NULL);

	// Initialize the conditional variables
	for (i = 0; i < N; i++) {
		pthread_cond_init(&chopstick[i], NULL);
	}

    // Create the five philosopher threads
    for(i = 0; i < N; i ++) {
        a[i] = i;
        pthread_create(&tid[i], NULL, philosopher, (void*)&a[i]);
    }

    // Join all five philosopher threads
    for(i = 0; i < N; i ++) {
        pthread_join(tid[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&mtx);

    // Destroy the conditional variables
    for(i = 0; i < N; i ++) {
        pthread_cond_destroy(&chopstick[i]);
    }
}

// Main philosopher method to execute 
void *philosopher(void *num) {
    int phil = *(int*)num;
    printf("Philosopher %d has entered room\n", phil);
    sleep(2);

    while(1) {
        // The philosopher pick up the chopsticks to eat
        pthread_mutex_lock(&mtx);
        // Change the enum state of philosopher to HUNGRY
        state[phil] = HUNGRY;
        // If the philosopher can pick up the adjacent chopsticks to eat then change the enum state of philosopher to EATING
        // else the philosopher is blocked by their conditional variable chopstick[phil]
        for(; state[phil] == HUNGRY && (state[phil] == EATING || state[(phil + 1) % N] == EATING); pthread_cond_wait(&chopstick[phil], &mtx));
        state[phil] = EATING;
        printf("Philosopher %d takes fork %d and %d\n", phil, phil, (phil + 1) % N);
        pthread_mutex_unlock(&mtx);
        
        // The philosopher eat
        eat(phil);

        // The philosopher put down the chopsticks
        pthread_mutex_lock(&mtx);
        // Change the enum state of philosopher to THINKING
        state[phil] = THINKING;
        // Signal the current left and right phillosphers to notify that they can eat now
        pthread_cond_signal(&chopstick[phil]);
        pthread_cond_signal(&chopstick[(phil + 1) % N]);
        printf("Philosopher %d puts fork %d and %d down\n", phil, (phil + 1) % N, phil);
        pthread_mutex_unlock(&mtx);
        
        // The philosopher think
        think(phil);
        sleep(1);        
    }
}

void eat(int phil) {
    printf("Philosopher %d is eating\n", phil);
}

void think(int phil) {
    printf("Philosopher %d is thinking\n", phil);
}