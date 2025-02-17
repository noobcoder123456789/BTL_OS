#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/*
* Filename: problem3.c
*/

#define PERMISSION 0644
#define MSG_KEY 0x123

struct message_buffer {
    long message_type;
    char message_text[200];
};

void INIT(void) {
    int message_queue_ID_1;
    int message_queue_ID_2;
    if((message_queue_ID_1 == msgget(MSG_KEY, PERMISSION | IPC_CREAT)) == -1
    || (message_queue_ID_2 == msgget(MSG_KEY, PERMISSION | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
}

void* WRITE_TO_PARENT(void*) {
    /* send parent a message through message queue */
    struct message_buffer buffer;
    key_t key; int len, message_queue_ID_2;
    system("touch temp.txt");        

    /* create the message queue for Child to write message */
    if((message_queue_ID_2 = msgget(MSG_KEY, PERMISSION | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Message Queue of Child: Ready To Send Message\n");
    printf("Enter Lines Of Child's Input Text, Enter \"end\" For Exit:\n\n");
    buffer.message_type = 1;

    while(fgets(buffer.message_text, sizeof(buffer.message_text), stdin) != NULL) {        
        len = strlen(buffer.message_text);
        /* remove new line at end, if it exists */
        if(buffer.message_text[len - 1] == '\n') {
            buffer.message_text[len - 1] = '\0';
        }

        /* len + 1 for '\0' */
        if(msgsnd(message_queue_ID_2, &buffer, len + 1, 0) == -1) {
            perror("msgsnd");
        }

        if(strcmp(buffer.message_text, "end") == 0) {
            printf("Message Queue of Child: Done Sending Messages.\n");
            printf("Message Queue of Parent: Done Receiving Messages.\n\n");            
            break;
        }
    }

    strcpy(buffer.message_text, "end");
    len = strlen(buffer.message_text);
    /* len + 1 for '\0' */
    if(msgsnd(message_queue_ID_2, &buffer, len + 1, 0) == -1) {
        perror("msgsnd");
    }

    /* Remove the message queue identifier */
    if(msgctl(message_queue_ID_2, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }    
}

void* READ_FROM_PARENT(void*) {
    /* read message sent by parent from message queue */
    struct message_buffer buffer;
    key_t key; int check, message_queue_ID_1;
    
    /* connect the message queue for child to read */ 
    if((message_queue_ID_1 = msgget(MSG_KEY, PERMISSION)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    
    printf("Message Queue of Child: Ready To Receive Messages\n");

    do {
        /* receiving message until the program ends with the string "end" */
        if(msgrcv(message_queue_ID_1, &buffer, sizeof(buffer.message_text), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        check = strcmp(buffer.message_text, "end");
        if(check) {
            printf("Child Received Message: \"%s\"\n\n", buffer.message_text);
        }
    } while(check);
    
    system("rm temp.txt");
}

void* WRITE_TO_CHILD(void*) {
    /* send child a message through message queue */
    struct message_buffer buffer;
    key_t key; int len, message_queue_ID_1;
    system("touch temp.txt");

    /* create the message queue */
    if((message_queue_ID_1 = msgget(MSG_KEY, PERMISSION | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Message Queue of Parent: Ready To Send Messages\n");
    printf("Enter Lines Of Parent's Input Text, Enter \"end\" For Exit:\n\n");
    buffer.message_type = 1;

    while(fgets(buffer.message_text, sizeof buffer.message_text, stdin) != NULL) {
        len = strlen(buffer.message_text);
        /* remove new line at end, if it exists */
        if(buffer.message_text[len - 1] == '\n') {
            buffer.message_text[len - 1] = '\0';
        }

        /* len + 1 for '\0' */
        if(msgsnd(message_queue_ID_1, &buffer, len + 1, 0) == -1) {
            perror("msgsnd");
        }
        
        if(strcmp(buffer.message_text, "end") == 0) {
            printf("Message Queue of Parent: Done Sending Messages.\n");
            printf("Message Queue of Child: Done Receiving Messages.\n\n");            
            break;
        }
    }

    strcpy(buffer.message_text, "end");
    len = strlen(buffer.message_text);
    /* len + 1 for '\0' */
    if(msgsnd(message_queue_ID_1, &buffer, len + 1, 0) == -1) {
        perror("msgsnd");
    }

    /* Remove the message queue identifier */
    if(msgctl(message_queue_ID_1, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }    
}

void* READ_FROM_CHILD(void*) {
    /* read the message sent by child from message queue */
    struct message_buffer buffer;
    key_t key; int check, message_queue_ID_2;

    /* connect the message queue */ 
    if((message_queue_ID_2 = msgget(MSG_KEY, PERMISSION | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    
    printf("Message Queue of Parent: Ready To Receive Messages\n");

    do {
        /* receiving message until the program ends with the string "end" */
        if(msgrcv(message_queue_ID_2, &buffer, sizeof(buffer.message_text), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        check = strcmp(buffer.message_text, "end");
        if(check) {
            printf("Parent Received Message: \"%s\"\n\n", buffer.message_text);        
        }
    } while(check);

    system("rm temp.txt");
}

int main(int argc, char *argv[]) {
    INIT();
    pthread_t parent_thread, child_thread;

    /* session 1: parent thread write and child thread read */
    pthread_create(&parent_thread, NULL, &WRITE_TO_CHILD, NULL);
    pthread_create(&child_thread, NULL, &READ_FROM_PARENT, NULL);
    pthread_join(parent_thread, NULL);
    pthread_join(child_thread, NULL);
    
    /* session 2: parent thread read and child thread write */
    pthread_create(&child_thread, NULL, &WRITE_TO_PARENT, NULL);
    pthread_create(&parent_thread, NULL, &READ_FROM_CHILD, NULL);
    pthread_join(parent_thread, NULL);
    pthread_join(child_thread, NULL);
    
    return EXIT_SUCCESS;
}