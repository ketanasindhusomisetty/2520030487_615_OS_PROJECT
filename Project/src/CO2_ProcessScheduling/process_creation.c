//Step 1 — fork() + wait()

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    printf("========================================\n");
    printf("       CO-2: PROCESS CREATION\n");
    printf("========================================\n\n");

    printf("[Parent] Parent process started.\n");
    printf("[Parent] Parent PID: %d\n\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");
        return 1;

    } else if (pid == 0) {

        /* Child Process */

        printf("[Child] Child process created successfully.\n");
        printf("[Child] Child PID : %d\n", getpid());
        printf("[Child] Parent PID: %d\n", getppid());

        printf("[Child] Child process is executing...\n");

        sleep(2);

        printf("[Child] Child process completed.\n");

        exit(0);

    } else {

        /* Parent Process */

        printf("[Parent] fork() created child process.\n");
        printf("[Parent] Child PID: %d\n", pid);

        printf("[Parent] Waiting for child process...\n");

        wait(NULL);

        printf("[Parent] Child process has completed.\n");
        printf("[Parent] Parent process continues execution.\n");
    }

    printf("\n========================================\n");
    printf("       PROCESS CREATION COMPLETE\n");
    printf("========================================\n");

    return 0;
}