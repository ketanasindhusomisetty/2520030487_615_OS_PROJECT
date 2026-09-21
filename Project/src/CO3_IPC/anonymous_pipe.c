//Step 1: Anonymous Pipe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main() {

    int pipefd[2];

    pid_t pid;

    char message[] =
        "Hello from Parent Process through Anonymous Pipe!";

    char buffer[100];


    printf("========================================\n");
    printf("       CO-3: ANONYMOUS PIPE IPC\n");
    printf("========================================\n\n");


    /*
     * Create the pipe.
     *
     * pipefd[0] -> Read end
     * pipefd[1] -> Write end
     */
    if (pipe(pipefd) == -1) {

        perror("pipe");
        return 1;
    }

    printf("[Parent] Anonymous pipe created successfully.\n");


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
        printf("[Child] PID: %d\n", getpid());

        /*
         * Child only reads.
         * Therefore close the write end.
         */
        close(pipefd[1]);

        /*
         * Read data from pipe.
         */
        ssize_t bytes_read =
            read(pipefd[0],
                 buffer,
                 sizeof(buffer) - 1);

        if (bytes_read < 0) {

            perror("read");
            close(pipefd[0]);
            exit(1);
        }

        buffer[bytes_read] = '\0';

        printf("[Child] Message received from parent:\n");
        printf("        \"%s\"\n", buffer);

        /*
         * Close read end.
         */
        close(pipefd[0]);

        printf("[Child] Read end of pipe closed.\n");

        exit(0);
    }


    /*
     * PARENT PROCESS
     */
    else {

        printf("[Parent] Parent PID: %d\n", getpid());
        printf("[Parent] Child PID : %d\n", pid);

        /*
         * Parent only writes.
         * Therefore close the read end.
         */
        close(pipefd[0]);

        /*
         * Write message into pipe.
         */
        ssize_t bytes_written =
            write(pipefd[1],
                  message,
                  strlen(message));

        if (bytes_written < 0) {

            perror("write");
            close(pipefd[1]);
            return 1;
        }

        printf("[Parent] Message sent through pipe.\n");
        printf("[Parent] Bytes written: %ld\n",
               (long)bytes_written);

        /*
         * Close write end.
         */
        close(pipefd[1]);

        printf("[Parent] Write end of pipe closed.\n");

        /*
         * Wait for child.
         */
        wait(NULL);

        printf("[Parent] Child process completed.\n");
    }


    printf("\n========================================\n");
    printf("       ANONYMOUS PIPE COMPLETE\n");
    printf("========================================\n");

    return 0;
}