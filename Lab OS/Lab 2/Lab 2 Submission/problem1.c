#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SHM_KEY 0x123

/* initialize shared memory pointer as NULL */
long* shared_memory_pointer = NULL;
/* 
    index 0 -> 2003 of shared_memory_pointer save the sum of rating of all user for each movie
    index 2004 -> 4007 of shared_memory_pointer save the number of user rate for each movie
*/

/* this function for assign value */
int compare_and_swap(int *value, int expected, int new_value) {
    int temp = *value;

    if(*value == expected) {
        *value = new_value;
    }

    return temp;
}

void read_file(char *fileName) {
    /* Open File */
    FILE *ptr = fopen(fileName, "r");

    /* check if can't open file */
    if(!ptr) {
        printf("File Can't Be Opened\n");
        return;
    }

    /* Read File */
    char ch; /* char for input */
    int flag_check_num = 0; /* flag_check_num equal 1 if meet number or equal 0 otherwise */
    int flag_check_space = 1; /* flag_check_space equal 1 if meet space or equal 0 otherwise */
    int read_num = 0; /* read_num use for getting input number */
    int movieID = 0; /* movieID use for getting movie ID */

    /* because of the format of input in file we only need to care about the second and the third number of each line */
    do {        
        ch = fgetc(ptr);

        /* if meet end of line */
        if(ch == '\n') {
            compare_and_swap(&movieID, movieID, 0); /* movieID = 0 */
            compare_and_swap(&read_num, read_num, 0); /* read_num = 0 */
            compare_and_swap(&flag_check_num, flag_check_num, 0); /* flag_check_num = 0 */
            compare_and_swap(&flag_check_space, flag_check_space, 1); /* flag_check_space = 0 */
        }
        
        /* if ch is number and have met space */
        if(ch >= '0' && ch <= '9' && flag_check_space == 1) {
            compare_and_swap(&flag_check_space, flag_check_space, 0); /* flag_check_space = 0 */
            compare_and_swap(&flag_check_num, flag_check_num, flag_check_num + 1); /* flag_check_num = flag_check_num + 1 */
        }

        /* if ch is not number and not also end of line */
        if(!(ch >= '0' && ch <= '9') && ch != '\n') {
            if(flag_check_num == 2) {
                compare_and_swap(&movieID, movieID, read_num); /* movieID = read_num */
                /* increment 1 for movieID */
                shared_memory_pointer[movieID + 2004]++;
            } else if(flag_check_num == 3) {
                /* sum rating for movieID */
                shared_memory_pointer[movieID] += read_num;
            }

            compare_and_swap(&read_num, read_num, 0); /* read_num = 0 */
            compare_and_swap(&flag_check_space, flag_check_space, 1); /* flag_check_space = 1 */
        }
        
        if((flag_check_num == 2 || flag_check_num == 3) && flag_check_space == 0) {
            compare_and_swap(&read_num, read_num, read_num * 10); /* read_num = read_num * 10 */
            compare_and_swap(&read_num, read_num, read_num + (ch - '0')); /* read_num = read_num + (ch - '0') */
        }
    } while(ch != EOF);
    
    printf("\n");

    /* Close File */
    fclose(ptr);
    return;
}

void generate_two_processes_to_do_seperate_task() {
    pid_t child_a, child_b;

    /* create process child_a */
    child_a = fork();

    /* check if can't create process child_a */
    if(child_a < 0) {
        perror("Fork Failed To Create Child Process A!\n");
        exit(1);
    } else if(child_a == 0) {
        /* create process child_b */
        child_b = fork();
        /* check if can't create process child_b */
        if(child_b < 0) {
            perror("Fork Failed To Create Child Process B!\n");
            exit(1);
        } else if(child_b == 0) { /* if process child_b is running*/
            read_file("movie-100k_2.txt");
        } else { /* if process child_a is running */
            read_file("movie-100k_1.txt");
        }
    } else {
        wait(NULL); /* wait for process child_a and child_b to complete */
        /* print the movieID and average rating for each movie */
        printf("MovieID    Average Rating\n");
        for(int i = 0; i < 2004; i ++) {
            if(shared_memory_pointer[i]) {
                float average_rating = shared_memory_pointer[i] / shared_memory_pointer[i + 2004];
                printf("%d", i);
                if(i <= 9) printf("          ");
                else if(i <= 99) printf("         ");
                else if(i <= 999) printf("        ");
                else printf("       ");
                printf("%.2f\n", average_rating);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    /* Create Shared Memory Region */
    int shmid = shmget(SHM_KEY, 4012 * sizeof(long), 0644 | IPC_CREAT);

    if(shmid < 0) {
        perror("\nShared Memory Error");
        return 1;
    } else {
        printf("\nShared Memory: %d\n", shmid);
    }

    /* Create Shared Segment */
    shared_memory_pointer = (long*)shmat(shmid, 0, 0);

    generate_two_processes_to_do_seperate_task();    

    /* Detach From The Shared Memory */
    if(shmdt(shared_memory_pointer) == -1) { 
        perror("shmdt");
        return 1;
    }

    return 0;
}