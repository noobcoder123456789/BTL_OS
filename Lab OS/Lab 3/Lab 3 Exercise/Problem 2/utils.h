
#define PACKAGE "aggsum"
#define VERSION "0.01"

#define SEEDNO 1024   /* default seed for randomizer */
#define ARG1 "<arrsz>"
#define ARG2 "<tnum>"
#define ARG3 "<seednum>"

#define THRSL_MIN 5      /* minimum threshold of computation size per thread */
#define UPBND_DATA_VAL 100 /* upper bound (maximum threshold) of generated data value*/
#define LWBND_DATA_VAL 0   /* lower bound (minimum threshold) of generated data value*/

int tonum (const char * nptr, int * num);
void help (int xcode);

struct _appconf {
  int arrsz;
  int tnum;
  int seednum;
} appconf;

struct _range {
  int start;
  int end;
};

int thread_num_global_val = -1;

/** process command line argument.
 *  values are made available through the 'conf' struct.
 *  using the parsed conf to get arguments: the arrsz, tnum, and seednum
 */
int processopts (int argc, char **argv, struct _appconf *conf) {
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
int tonum (const char * nptr, int * num) {
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
int validate_and_split_argarray (int arraysize, int num_thread, struct _range* thread_idx_range) {
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
int generate_array_data (int* buf, int arraysize, int seednum) {
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

/* display help */
void help (int xcode) {
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