#ifndef BKTPOOL_H
#define BKTPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_WORKER 10
#define WRK_THREAD 1
#define STACK_SIZE 4096
#define SIG_DISPATCH SIGUSR1
#define SHM_KEY 0x1234
#define SHM_SIZE sizeof(struct shared_memory)

pthread_mutex_t mtx;

/* Task ID is unique non-decreasing integer */
int taskid_seed;

// int wrkid_tid[MAX_WORKER];
// int wrkid_busy[MAX_WORKER];
// int wrkid_cur;

struct bkworker_t; // Forward declaration
struct bktask_t {
  void( * func)(void * arg);
  void * arg;
  unsigned int bktaskid;
  struct bktask_t * tnext;
}* bktask;

int bktask_sz;

struct bkworker_t {
  void( * func)(void * arg);
  void * arg;
  unsigned int wrkid;
  unsigned int bktaskid;
};

//struct bkworker_t worker[MAX_WORKER];

struct shared_memory {
  struct bkworker_t worker[MAX_WORKER];
  int wrkid_tid[MAX_WORKER];
  int wrkid_busy[MAX_WORKER];
  int wrkid_cur;
  int wrk_args[MAX_WORKER];
};

struct shared_memory *shared_data;

/* Prototype API */

/* bktpool module */
int bktpool_init();

/* bktask module */
struct bktask_t * bktask_get_byid(unsigned int bktaskid);
int bktask_init(unsigned int * bktaskid, void * func, void * arg);
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid);

/* bkwrk module */
void * bkwrk_worker(void * arg);
int bkwrk_create_worker();
int bkwrk_dispatch_worker(unsigned int wrkid);
int bkwrk_get_worker();

#endif