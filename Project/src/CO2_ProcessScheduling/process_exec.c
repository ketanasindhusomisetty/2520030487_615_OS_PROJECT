//Step 2 - fork() + exec()

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    printf("========================================\n");
    printf("          CO-2: FORK + EXEC\n");
    printf("========================================\n\n");

    printf("[Parent] Parent process started.\n");
    printf("[Parent] Parent PID: %d\n\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork");
        return 1;

    } else if (pid == 0) {

        /* Child Process */

        printf("[Child] Child process created.\n");
        printf("[Child] Child PID: %d\n", getpid());

        printf("[Child] Executing 'ls -l' using execl().\n\n");

        /*
         * execl() replaces the current child
         * process with the 'ls' program.
         */
        execl("/bin/ls", "ls", "-l", NULL);

        /*
         * This line executes only if execl()
         * fails.
         */
        perror("execl");
        exit(1);

    } else {

        /* Parent Process */

        printf("[Parent] Child PID: %d\n", pid);
        printf("[Parent] Waiting for child...\n\n");

        wait(NULL);

        printf("\n[Parent] Child completed.\n");
        printf("[Parent] Parent continues execution.\n");
    }

    printf("\n========================================\n");
    printf("          FORK + EXEC COMPLETE\n");
    printf("========================================\n");

    return 0;
}