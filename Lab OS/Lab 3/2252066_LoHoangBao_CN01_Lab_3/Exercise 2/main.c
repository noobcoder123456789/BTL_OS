#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for getopt */

#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <pthread.h>

/** process command line argument.
 *  values are made available through the 'conf' struct.
 *  using the parsed conf to get arguments: the arrsz, tnum, and seednum
 */
extern int processopts (int argc, char **argv, struct _appconf *conf) {
  int exit_val = 0;
  conf->arrsz = atoi(argv[1]);
  conf->tnum = atoi(argv[2]);
  thread_num_global_val = conf->tnum;

  if(conf->arrsz < 0) {
    printf("ERROR: Invalid value for array size variable.\n");
    help(EXIT_SUCCESS);
    exit_val++;
  }

  if(conf->tnum < 0) {
    printf("ERROR: Invalid value for number of thread.\n");
    help(EXIT_SUCCESS);
    exit_val++;
  }

  if(argc == 3) {
    conf->seednum = SEEDNO;
  } else if(argc == 4 && atoi(argv[3]) < 0) {
    printf("ERROR: Invalid value for seed number.\n");
    help(EXIT_SUCCESS);
    exit_val++;
  }

  return exit_val;
}

/** process string to number.
 *  string is stored in 'nptr' char array.
 *  'num' is returned the valid integer number.
 *  return 0 valid number stored in num
 *        -1 invalid and num is useless value.
 */
extern int tonum (const char * nptr, int * num) {
  *num = 0; int check = 0;

  for(size_t i = 0; i < strlen(nptr); i ++) {
    if(!(nptr[i] >= '0' && nptr[i] <= '9')) {
      check = 1; break;
    }

    *num = (*num * 10) + (nptr[i] - '0');
  }

  if(check == 1) {
    return -1;
  }

  return 0;
}

/** validate the array size argument.
 *  the size must be splitable "num_thread".
 */
extern int validate_and_split_argarray (int arraysize, int num_thread, struct _range* thread_idx_range) {
  if(arraysize % num_thread != 0) {
    return -1;
  }

  int jump = arraysize / num_thread;
  for(int i = 0; i < num_thread; i ++) {    
    thread_idx_range[i].start = i * jump;
    thread_idx_range[i].end = (i + 1) * jump - 1;
  }

  return 0;
}

/** generate "arraysize" data for the array "buf"
 *  validate the array size argument.
 *  the size must be splitable "num_thread".
 */
extern int generate_array_data (int* buf, int arraysize, int seednum) {
  printf("%d\n\n", thread_num_global_val);
  if(arraysize % thread_num_global_val != 0) {
    return -1;
  }

  srand(seednum);
  
  for(int i = 0; i < arraysize; i ++) {
    buf[i] = (rand() % (UPBND_DATA_VAL - LWBND_DATA_VAL + 1)) + LWBND_DATA_VAL;
  }

  return 0;
}

/** display help */
extern void help (int xcode) {
  xcode = xcode ? xcode : 0;
  printf("aggsum, version %s\n\n", VERSION);
  printf("usage: %s arrsz tnum [seednum]\n\n", PACKAGE);
  printf("Generate randomly integer array size %s and calculate sum parallelly\n", ARG1);
  printf("using %s threads. The optional <seednum> value use to control the\n", ARG2);
  printf("randomization of the generated array.\n\n");
  printf("Arguments:\n\n");
  printf("    arrsz      specifies the size of array.\n");
  printf("    tnum       number of parallel threads.\n");
  printf("    seednum    initialize the seed of the randomized generator srand().\n");
  exit(xcode);
}

void* sum_worker (struct _range *idx_range);
long validate_sum(int arraysize);

/* Global sum buffer */
long sumbuf = 0;
int* shrdarrbuf;
pthread_mutex_t mtx;

void* sum_worker (struct _range *idx_range) {
   int i;
   
   pthread_mutex_lock(&mtx);
   // printf("In worker from %d to %d\n", idx_range->start, idx_range->end);      
   for(i = idx_range->start; i <= idx_range->end; sumbuf += shrdarrbuf[i], i ++);
   pthread_mutex_unlock(&mtx);
   
   return 0;
		
}

int main(int argc, char * argv[]) {
   int i, arrsz, tnum, seednum;
   char *buf;
   struct _range* thread_idx_range;
   pthread_t* tid;
   int pid;

   if (argc < 3 || argc > 4) {/* only accept 2 or 3 arguments */
      help(EXIT_SUCCESS);
   }

#if(DBGSTDERR == 1)
   freopen("/dev/null","w",stderr); /* redirect stderr by default */
#endif
   processopts(argc, argv, &appconf); /* process all option and argument */

   fprintf(stdout,"%s runs with %s=%d \t %s=%d \t %s=%d\n", PACKAGE, ARG1, appconf.arrsz, ARG2, appconf.tnum, ARG3, appconf.seednum);

   thread_idx_range = malloc(appconf.tnum * sizeof(struct _range));
   if(thread_idx_range == NULL) {
      printf("Error! memory for index storage not allocated.\n");
      exit(-1);
   }
	
   if (validate_and_split_argarray(appconf.arrsz, appconf.tnum, thread_idx_range) < 0) {
      printf("Error! array index not splitable. Each partition need at least %d item\n", THRSL_MIN);
      exit(-1);	   
   }
    
   /* Generate array data */
   shrdarrbuf = malloc(appconf.arrsz * sizeof(int));
   if(shrdarrbuf == NULL) {
      printf("Error! memory for array buffer not allocated.\n");
      exit(-1);
   }
 
   if(generate_array_data(shrdarrbuf, appconf.arrsz, appconf.seednum) < 0) {
      printf("Error! array index not splitable.\n");
      exit(-1);	   
   }

   pid=fork();
   
   if(pid < 0) {
      printf("Error! fork failed.\n");
      exit(-1);   
   }  
   
   if(pid == 0) { /* child process do a validation sum */
      printf("sequence sum results %ld\n",validate_sum(appconf.arrsz));
      exit(0);
   } // parent process goes here
	   
   /** Create <tnum> thead to calculate partial non-volatile sum
    *  the non-volatile mechanism require value added to global sum buffer
    */

   pthread_mutex_init(&mtx, NULL);

   tid = malloc (appconf.tnum * sizeof(pthread_t));

   for (i=0; i < appconf.tnum; i++) {
      pthread_create(&tid[i], NULL, (void *)sum_worker, (
                     struct _range *) (&thread_idx_range[i]));
   }

   for (i=0; i < appconf.tnum; i++) {
      pthread_join(tid[i], NULL);
   }

   fflush(stdout);
	
   printf("%s gives sum result %ld\n", PACKAGE, sumbuf);

   waitpid(pid);
   exit(0);
}

long validate_sum(int arraysize)
{
   long validsum = 0;
   int i;

   for (i=0; i < arraysize; i++) {
      validsum += shrdarrbuf[i];
   }

   return validsum;
}
