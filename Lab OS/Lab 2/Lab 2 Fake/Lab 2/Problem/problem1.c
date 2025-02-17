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

long* shared_memory_pointer = NULL;

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

    if(!ptr) {
        printf("File Can't Be Opened\n");
        // perror("File Can't Be Opened\n");
        return;
    }

    // printf("Content Of This File Are\n");

    /* Read File */
    char ch;
    int flag_check_num = 0, flag_check_space = 1;
    int read_num = 0, movieID = 0;

    do {        
        ch = fgetc(ptr);

        if(ch == '\n') {
            compare_and_swap(&movieID, movieID, 0);
            compare_and_swap(&read_num, read_num, 0);
            compare_and_swap(&flag_check_num, flag_check_num, 0);
            compare_and_swap(&flag_check_space, flag_check_space, 1);
        }
        
        if(ch >= '0' && ch <= '9' && flag_check_space == 1) {
            compare_and_swap(&flag_check_space, flag_check_space, 0);
            compare_and_swap(&flag_check_num, flag_check_num, flag_check_num + 1);            
        }

        if(!(ch >= '0' && ch <= '9') && ch != '\n') {
            if(flag_check_num == 2) {
                compare_and_swap(&movieID, movieID, read_num);
                shared_memory_pointer[movieID + 2004]++;
            } else if(flag_check_num == 3) {
                shared_memory_pointer[movieID] += read_num;
            }

            compare_and_swap(&read_num, read_num, 0);
            compare_and_swap(&flag_check_space, flag_check_space, 1);            
        }

        if((flag_check_num == 2 || flag_check_num == 3) && flag_check_space == 0) {
            compare_and_swap(&read_num, read_num, read_num * 10);
            compare_and_swap(&read_num, read_num, read_num + (ch - '0'));
        }

        // printf("%c", ch);
    } while(ch != EOF);
    
    printf("\n");

    /* Close File */
    fclose(ptr);
    return;
}

void generate_two_processes_to_do_seperate_task() {
    pid_t child_a, child_b;

    child_a = fork();

    if(child_a < 0) {
        perror("Fork Failed To Create Child Process A!\n");
        exit(1);
    } else if(child_a == 0) {        
        child_b = fork();
        if(child_b < 0) {
            perror("Fork Failed To Create Child Process B!\n");
            exit(1);
        } else if(child_b == 0) {
            read_file("movie-100k_2.txt");
        } else {
            read_file("movie-100k_1.txt");
        }
    } else {
        wait(NULL);
        // printf("Hello From Parent Process!\n");

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
        perror("Shared Memory Error");
        return 1;
    } else {
        printf("Shared Memory: %d\n", shmid);
    }

    /* Create Shared Segment */
    shared_memory_pointer = (long*)shmat(shmid, 0, 0);

    generate_two_processes_to_do_seperate_task();    

    /* Detach From The Shared Memory */
    if(shmdt(shared_memory_pointer) == -1) { 
        perror("shmdt");
        return 1;
    }

    // /* Mark The Shared Segment To Be Destroyed */
    // if(shmctl(shmid, IPC_RMID, (void*)0) == -1) {
    //     perror("shmctl");
    //     return 1;
    // }

    return 0;
}