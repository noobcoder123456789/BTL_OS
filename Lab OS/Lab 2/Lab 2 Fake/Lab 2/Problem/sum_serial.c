#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: sum_serial.out <integer value>\n");
        return -1;
    }

    int n = atoi(argv[1]);
    if(n <= 0) {
        fprintf(stderr, "%d must be > 0\n", n);
        return -1;
    }

    long sum = 0;
    for(int i = 1; n >= i; sum += i, i ++);
    if(n == 1) printf("sum(1) = 1\n");
    else if(n == 2) printf("sum(2) = 1 + 2 = 3\n");
    else printf("sum(%d) = 1 + 2 + .... + %d = %ld\n", n, n, sum);
    return 0;
}