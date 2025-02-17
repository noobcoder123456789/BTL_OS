#include <stdio.h>
#include <unistd.h>

int main() {
    int pipefds[2];
    int returnsatus;
    int pid;
    char writemessages[20] = "Hello";
    char readmessage[20];
    returnsatus = pipe(pipefds);
    
    if(returnsatus == -1) {
        printf("Unable To Create Pipe\n");
        return 1;
    }

    pid = fork();

    // Child Process
    if(!pid) {
        read(pipefds[0], readmessage, sizeof(readmessage));
        printf("Child Process: Reading, message is %s \n", readmessage);
        return 0;
    }

    // Parent Process
    printf("Parent Process: Writing, message is %s \n", writemessages);
    write(pipefds[1], writemessages, sizeof(writemessages));
    return 0;
}