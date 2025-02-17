#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>

/* function for checking if file is exist or not */
bool file_exist(const char *file_name) {
    FILE *file;
    if((file = fopen(file_name, "r"))) {
        fclose(file);
        return true;
    }
    
    return false;
}

void WRITE(void *mapped_memory) {
    /* initialize the memory-mapped region with data */    
    char message[200];
    fgets(message, sizeof(message), stdin);
    strncpy(mapped_memory, message, strlen(message));    
}

void READ(void *mapped_memory) {
    /* read the data in memory-mapped region */
    printf("Contents of the memory-mapped region: %s\n", (char*)mapped_memory);
}

int main(int argc, char *argv[]) {        
    const char *file_path = "mapping_created_file.txt"; /* initialize file path */    
    const size_t file_size = 4096; /* initialize file size */
    int file_descriptor;
    
    if(!file_exist(file_path)) { /* open the file if it already exist
                                if it's not exist then create and open it */    
        file_descriptor = open(file_path, O_CREAT | O_RDWR, 0400 | 0200);
    }

    if (file_descriptor == -1) {
        perror("open");
        return 1;
    }
 
    if (ftruncate(file_descriptor, file_size) == -1) {
        perror("ftruncate");
        close(file_descriptor);
        return 1;
    }
    
    void *mapped_memory = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0); /* create a memory-mapped region for the file */

    /* check error for mmap() */
    if(mapped_memory == MAP_FAILED) {
        perror("mmap");
        close(file_descriptor);
        if(file_exist("mapping_created_file.txt")) { /* check if file exist to remove */
            system("rm mapping_created_file.txt"); /* remove file */
        }
        return EXIT_FAILURE;
    }
     
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork");
        close(file_descriptor);
        if(file_exist("mapping_created_file.txt")) { /* check if file exist to remove */
            system("rm mapping_created_file.txt"); /* remove file */
        }
        return EXIT_FAILURE;
    } else if(pid == 0) {
        printf("Enter Lines Of Child's Input Text: ");
        WRITE(mapped_memory);
        READ(mapped_memory);
    } else {
        wait(NULL);
        printf("Enter Lines Of Parent's Input Text: ");
        WRITE(mapped_memory);
        READ(mapped_memory);        
    }
    
    wait(NULL);    
    if(file_exist("mapping_created_file.txt")) { /* check if file exist to remove */
        system("rm mapping_created_file.txt"); /* remove file */
    }
    munmap(mapped_memory, file_size); /* unmap the memory and close the file */
    close(file_descriptor);    
    return EXIT_SUCCESS;
}