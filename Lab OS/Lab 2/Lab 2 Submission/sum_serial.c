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
    if(argc != 2) { /* check if the number of argument is not 2 */
        fprintf(stderr, "Usage: sum_serial.out <integer value>\n");
        return -1;
    }

    int n = atoi(argv[1]); /* n is the number of thread*/
    if(n <= 0) { /* check if n is negative then notify */
        fprintf(stderr, "%d must be > 0\n", n);
        return -1;
    }

    long sum = 0; /* sum for calculate sum(n) */
    for(int i = 1; n >= i; sum += i, i ++); /* calculate the sum(n) */
    if(n == 1) printf("sum(1) = 1\n");
    else if(n == 2) printf("sum(2) = 1 + 2 = 3\n");
    else printf("sum(%d) = 1 + 2 + .... + %d = %ld\n", n, n, sum);
    return 0;
}