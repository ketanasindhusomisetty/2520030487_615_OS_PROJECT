//Step 3: Signals & Signal Handlers

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Signal handler for SIGUSR1.
 *
 * The handler uses write() because it is
 * safe to use inside a signal handler.
 */
void handle_sigusr1(int signal_number) {

    const char message[] =
        "[Child] SIGUSR1 received! Signal handler executed.\n";

    write(STDOUT_FILENO,
          message,
          sizeof(message) - 1);
}


int main() {

    pid_t pid;

    printf("========================================\n");
    printf("        CO-3: SIGNAL IPC DEMO\n");
    printf("========================================\n\n");

    /*
     * Register the signal handler.
     */
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {

        perror("signal");
        return 1;
    }

    printf("[Parent] Parent PID: %d\n", getpid());

    /*
     * Create child process.
     */
    pid = fork();

    if (pid < 0) {

        perror("fork");
        return 1;
    }


    /*
     * CHILD PROCESS
     */
    else if (pid == 0) {

        printf("[Child] Child process created.\n");
        printf("[Child] Child PID: %d\n", getpid());

        printf("[Child] Waiting for SIGUSR1...\n");

        /*
         * Wait for a signal.
         */
        pause();

        printf("[Child] Signal handling completed.\n");

        exit(0);
    }


    /*
     * PARENT PROCESS
     */
    else {

        printf("[Parent] Child PID: %d\n", pid);

        /*
         * Give the child time to enter pause().
         */
        sleep(2);

        printf("\n[Parent] Sending SIGUSR1 to child...\n");

        /*
         * Send SIGUSR1 to child.
         */
        if (kill(pid, SIGUSR1) == -1) {

            perror("kill");
            return 1;
        }

        printf("[Parent] SIGUSR1 sent successfully.\n");

        /*
         * Wait for child to finish.
         */
        wait(NULL);

        printf("[Parent] Child process completed.\n");
    }


    printf("\n========================================\n");
    printf("          SIGNAL DEMO COMPLETE\n");
    printf("========================================\n");

    return 0;
}