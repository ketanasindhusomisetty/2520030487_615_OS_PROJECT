//User Space & Kernel Space

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {

    printf("========================================\n");
    printf("       CO-1: USER SPACE & KERNEL SPACE\n");
    printf("========================================\n\n");

    printf("1. Program is currently executing in User Space.\n");

    printf("\n2. Requesting information from the Linux kernel...\n");

    pid_t pid = getpid();

    printf("3. System Call: getpid()\n");
    printf("4. Kernel returned the Process ID: %d\n", pid);

    printf("\n----------------------------------------\n");
    printf("User Space  ->  System Call  ->  Kernel\n");
    printf("----------------------------------------\n");

    printf("\nThe program requested an OS service through\n");
    printf("the system call interface.\n");

    printf("\n========================================\n");
    printf("        CO-1 STEP 2 COMPLETED\n");
    printf("========================================\n");

    return 0;
}