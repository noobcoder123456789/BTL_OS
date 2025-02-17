#include<stdio.h>
#include<unistd.h>
#include<sys/mman.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<signal.h>


#define MAX_WORKER 5
#define STACK_SIZE 1024

int bkwrk_worker(void *arg) {
    int *shared_data = (int *)arg;
    printf("Child process %d started with shared data: %d\n", getpid(), *shared_data);
    (*shared_data)++;  // Modify the shared data
    printf("Child process %d updated shared data to: %d\n", getpid(), *shared_data);
    return 0;
}

int bkwrk_create_worker_fork() {
    int i;
    pid_t pid;
    int *shared_data;

    // Create shared memory area
    shared_data = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    *shared_data = 0;  // Initialize shared data

    for (i = 0; i < MAX_WORKER; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process
            bkwrk_worker(shared_data);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            
            printf("Parent created child %d\n", pid);
            wait(NULL);
            
        } else {
            // Fork failed
            perror("fork");
            return -1;
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    printf("All children have finished. Final shared data value: %d\n", *shared_data);

    // Cleanup
    munmap(shared_data, sizeof(int));
    return 0;
}

int main(){
    bkwrk_create_worker_fork() ;
    return 0;
}
