#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMTASK 10
#define MAXWORKER 10
#define SHARED_MEM_SIZE (sizeof(int) * NUMTASK)

typedef struct{
    int a, b;
    int tid;
} Task;

pthread_mutex_t mtx;
pthread_cond_t waited;

// Task taskQueue[NUMTASK];
//int taskcnt = 0;
sem_t *mutex;
sem_t *tasks;

typedef struct {
    int sum;  
    int taskcnt;
    Task taskQueue[NUMTASK];
    pid_t pids[MAXWORKER];
    int workid;
} process;

process *shared_data;

void executeTask(Task *t){
    int result = t->a + t->b;
    printf("the sum of %d and %d is %d\n", t->a, t->b, t->a + t->b);
    shared_data->sum += result;
}

void submitTask(Task task){
    pthread_mutex_lock(&mtx);
    shared_data->taskQueue[shared_data->taskcnt] = task;
    //shared_data->worker[]

    shared_data->taskcnt++;
    
    pthread_mutex_unlock(&mtx);
    pthread_cond_signal(&waited);
}

void *startProcess(){
        Task task;
        
        pthread_mutex_lock(&mtx);
        while(shared_data->taskcnt == 0) {
            pthread_cond_wait(&waited, &mtx);
        }
        
        task = shared_data->taskQueue[0];
        
        for(int i = 0; i < NUMTASK; i++){
            sem_wait(mutex);
            shared_data->taskQueue[i] = shared_data->taskQueue[i + 1];
            sem_post(mutex);
            if(i == shared_data->taskcnt - 1 && shared_data->workid == MAXWORKER){
                printf("release all tasks, the current sum is %d\n", shared_data->sum);
                printf("\n");
                sem_wait(mutex);
                shared_data->workid = 0;
                sem_post(mutex);
            }
        }
        sem_wait(mutex);
        shared_data->workid++;
        printf("worker %d do task %d above\n", shared_data->workid, shared_data->taskQueue[NUMTASK % MAXWORKER].tid);
        sem_post(mutex);
        shared_data->taskcnt--;
        
        pthread_mutex_unlock(&mtx);
        executeTask(&task);
        
        printf("the count is %d\n", shared_data->taskcnt);
    
}

void process_init() {

    int shm_fd = shm_open("/data_shared", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, SHARED_MEM_SIZE) == -1) {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }
    
    shared_data = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUMTASK; i++) {
        shared_data->taskQueue[i].a = 0;
        shared_data->taskQueue[i].b = 0;
        shared_data->taskQueue[i].tid = 0;
    }
    shared_data->sum = 0;
    shared_data->taskcnt = 0;
    shared_data->workid = 0;
}

void process_fork(){
    for (int i = 0; i < NUMTASK; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            startProcess();
            printf("\n");
            // sleep(1);
            exit(EXIT_SUCCESS);

        } else {
            shared_data->pids[i] = pid;
            wait(NULL);
        }
    }
} 

void waiting(){
    for (int i = 0; i < MAXWORKER; i++) {
        waitpid(shared_data->pids[i], NULL, 0);
    }
}

int main(){
    process p;
    p.taskcnt = 0;
    srand(time(NULL));
    pthread_mutex_init(&mtx, NULL);

    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    tasks = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(mutex,1,1);
    //sem_init(tasks,1,MAXWORKER);

    pthread_cond_init(&waited, NULL);
    process_init();
    
    for(int i = 0 ;i < NUMTASK; i++){
        Task task; 
        task.a = i;//rand() % 100; 
        task.b = i;//rand() % 100;
        task.tid = i;
        submitTask(task);
        
    }

    // for(int i = 0 ;i < MAXWORKER; i++){
    //     printf("%d ", (shared_data->pids[i]));
    // }
   
    process_fork();
    waiting();

    pthread_cond_destroy(&waited);
    pthread_mutex_destroy(&mtx);
    sem_destroy(mutex);
    
    printf("the sum of all tasks is %d\n", shared_data->sum);
    
    return 0;
}