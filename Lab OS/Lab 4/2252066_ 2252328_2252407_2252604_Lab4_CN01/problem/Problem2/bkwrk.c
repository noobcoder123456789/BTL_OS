#include "bktpool.h"
#include <signal.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

//#define DEBUG
#define INFO
#define WORK_THREAD

int pipefd[MAX_WORKER][2];
pid_t worker_pids[MAX_WORKER];

// struct worker_argument_t {
//   void *arg;
//   int pipefd;
// };

void * bkwrk_worker(void * arg) {
#ifdef WORK_THREAD
  sigset_t set;
  int sig;
  int s;
  int i = * ((int * ) arg); // Default arg is integer of workid
  struct bkworker_t * wrk = &shared_data->worker[i];

  /* Taking the mask for waking up */
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGQUIT);

#ifdef DEBUG
  fprintf(stderr, "worker %i start living tid %d \n", i, getpid());
  fflush(stderr);
#endif

  while (1) {
    /* wait for signal */
    s = sigwait(&set, & sig);
    if (s != 0)
      continue;
  
#ifdef INFO
    fprintf(stderr, "worker wake %d up\n", i);
#endif

    /* Busy running */
    if (wrk -> func != NULL){
      wrk -> func((void *)&shared_data->wrk_args[i]);
    }
    /* Advertise I DONE WORKING */

    shared_data->wrkid_busy[i] = 0;
    shared_data->worker[i].func = NULL;
    shared_data->worker[i].arg = NULL;
    shared_data->wrk_args[i] = -1;
    shared_data->worker[i].bktaskid = -1;

    sleep(1);
  } 
#endif
}

int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid) {  
  if (wrkid < 0 || wrkid > MAX_WORKER) 
    return -1;
  struct bktask_t * tsk = bktask_get_byid(bktaskid);

  if (tsk == NULL) {    
    return -1;
  }    

  /* Advertise I AM WORKING */
  // wrkid_busy[wrkid] = 1;

  // worker[wrkid].func = tsk -> func;
  // worker[wrkid].arg = tsk -> arg;
  // worker[wrkid].bktaskid = bktaskid;

  shared_data->wrkid_busy[wrkid] = 1;

  shared_data->worker[wrkid].func = tsk->func;
  shared_data->worker[wrkid].arg = tsk->arg;
  shared_data->wrk_args[wrkid] = *((int *)tsk->arg);

  printf("Assign tsk %d wrk %d \n", tsk -> bktaskid, wrkid);
  sleep(1);
  return 0;
}

int bkwrk_create_worker() {
  unsigned int i;

  for (i = 0; i < MAX_WORKER; i++) {
#ifdef WORK_THREA
    void ** child_stack = (void ** ) malloc(STACK_SIZE);
    unsigned int wrkid = i;
    pthread_t threadid;

    sigset_t set;
    int s;

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    /* Stack grow down - start at top*/
    void * stack_top = child_stack + STACK_SIZE;
    
    
    shared_data->wrkid_busy[i] = 0;
    shared_data->worker[i].wrkid = i;
    shared_data->wrkid_tid[i] = clone( & bkwrk_worker, stack_top,
      CLONE_VM | CLONE_FILES,
      (void * ) & i);
      //printf("wrkid_tid[%d] = %d\n",i, wrkid_tid[i]);
#ifdef INFO
    fprintf(stderr, "bkwrk_create_worker got worker %u\n", shared_data->wrkid_tid[i]);
#endif

    //usleep(100);

#else
    
    sigset_t set;
    int s;

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    pid_t pid = fork();
    if (pid == 0) {
      bkwrk_worker(&i);
      exit(0);
    } else if (pid > 0) {
      shared_data->wrkid_tid[i] = pid;
      fprintf(stderr, "bkwrk_create_worker got worker %u\n",
              shared_data->wrkid_tid[i]);

      usleep(1);
    }
#endif
  }

  return 0;
}

int bkwrk_get_worker() {
  /* TODO Implement the scheduler to select the resource entity 
   * The return value is the ID of the worker which is not currently 
   * busy or wrkid_busy[1] == 0 
   */
  // pthread_mutex_lock(&mtx);
  // int retID = -1;
  // for(int idit = 0; idit < MAX_WORKER; idit++) {
  //   if(shared_data->wrkid_busy[idit] != 0) continue;
  //   retID = shared_data->worker[idit].wrkid;
  //   break;
  // }

  // shared_data->wrkid_cur = retID;
  // pthread_mutex_unlock(&mtx);
  // return retID;
  for (int i = 0; i < MAX_WORKER; i++) {
    if (shared_data->wrkid_busy[i] == 0) {
      return i;
    }
  }
  return -1;
}

int bkwrk_dispatch_worker(unsigned int wrkid) {
#ifdef WORK_THREA
  unsigned int tid = shared_data->wrkid_tid[wrkid];
  // /unsigned int tid = &shared_data->worker[wrkid];

  /* Invalid task */
  if (shared_data->worker[wrkid].func == NULL)
    return -1;

#ifdef DEBUG
  fprintf(stderr, "brkwrk dispatch wrkid %d - send signal %u \n", wrkid, tid);
#endif

  syscall(SYS_tkill, tid, SIG_DISPATCH);
#else
  /* TODO: Implement fork version to signal worker process here */ 
  pid_t pid = shared_data->wrkid_tid[wrkid];

  /* Invalid task */
  if (shared_data->worker[wrkid].func == NULL) {
    return -1;
  }

  kill(pid, SIG_DISPATCH);

  return 0;
#endif
}