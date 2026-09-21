//Step 3 - Process Lifecycle

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    printf("========================================\n");
    printf("       CO-2: PROCESS LIFECYCLE\n");
    printf("========================================\n\n");

    printf("[Parent] Process is CREATED.\n");
    printf("[Parent] PID: %d\n\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");
        return 1;

    } 
    else if (pid == 0) {

        /* Child Process */

        printf("[Child] Process CREATED.\n");
        printf("[Child] PID: %d\n", getpid());

        printf("[Child] State: READY\n");
        sleep(1);

        printf("[Child] State: RUNNING\n");

        printf("[Child] Performing some work...\n");
        sleep(3);

        printf("[Child] Work completed.\n");
        printf("[Child] State: TERMINATED\n");

        exit(0);

    } 
    else {

        /* Parent Process */

        printf("[Parent] Child process created.\n");
        printf("[Parent] Child PID: %d\n", pid);

        printf("[Parent] Waiting for child termination...\n");

        wait(NULL);

        printf("\n[Parent] Child process has TERMINATED.\n");
        printf("[Parent] Parent continues execution.\n");
    }

    printf("\n========================================\n");
    printf("       PROCESS LIFECYCLE COMPLETE\n");
    printf("========================================\n");

    return 0;
}