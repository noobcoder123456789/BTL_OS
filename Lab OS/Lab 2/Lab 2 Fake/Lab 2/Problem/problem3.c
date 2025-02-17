#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* pipefd1 for Parent to Write */
static int pipefd1[2], pipefd2[2];
/* pipefd2 for Child to Write */

void INIT(void) {
    if(pipe(pipefd1) < 0 || pipe(pipefd2) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
}

void WRITE_TO_PARENT(void) {
    /* send parent a message through pipe */
    char write_message[100] = "Hello Parent!";
    printf("Child send message to parent!\n");    
    write(pipefd2[1], write_message, sizeof(write_message));
    return;
}

void READ_FROM_PARENT(void) {
    /* read message sent by parent from pipe */
    char read_message[100];
    read(pipefd1[0], read_message, sizeof(read_message));
    printf("Child receive message from parent!\n");        
    printf("The message is: %s\n\n", read_message);
    return;
}

void WRITE_TO_CHILD(void) {
    /* send child a message through pipe */
    char write_message[100] = "Hello Child!";
    printf("Parent send message to child!\n");
    write(pipefd1[1], write_message, sizeof(write_message));
    return;    
}

void READ_FROM_CHILD(void) {
    /* read the message sent by child from pipe */
    char read_message[100];
    read(pipefd2[0], read_message, sizeof(read_message));
    printf("Parent receive message from child!\n");    
    printf("The message is: %s\n\n", read_message);
    return;    
}

int main(int argc, char *argv[]) {
    INIT();
    pid_t pid;
    pid = fork();
    // set a timer, process will end after 1 second.    
    alarm(10);
    if(pid == 0) {        
        while(1) {
            sleep(rand() % 2 + 1);
            WRITE_TO_CHILD();
            READ_FROM_CHILD();
        }        
    } else {
        while(1) {
            sleep(rand() % 2 + 1);            
            READ_FROM_PARENT();
            WRITE_TO_PARENT();
        }
    }

    return 0;
}