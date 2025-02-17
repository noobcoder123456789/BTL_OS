#include <stdio.h>
#include <unistd.h>
int x;
int y=10;

void print_func(int n) {
    int local_f;
    local_f = n;
    
    printf("\n Address of local_f: %p", &local_f);
    local_f--;
    if (local_f > 0) 
        print_func(local_f);
}

int main(int argc, char** argv) 
{
    printf("\n Process ID: %d \n", getpid());
    printf("\n Address of main function: %p", main);
    printf("\n Address of main arguments: %p , %p ", &argc, &argv);
    print_func(10);
    return 0;
}