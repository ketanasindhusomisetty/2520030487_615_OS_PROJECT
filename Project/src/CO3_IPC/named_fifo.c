//Step 2: Named Pipe (FIFO)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#define FIFO_NAME "/tmp/priority_scheduler_fifo"

int main() {

    pid_t pid;

    char message[] =
        "Priority Scheduling data sent through Named FIFO.";

    char buffer[200];

    printf("========================================\n");
    printf("          CO-3: NAMED FIFO IPC\n");
    printf("========================================\n\n");


    /*
     * Create the Named FIFO.
     *
     * If the FIFO already exists, unlink()
     * removes the old FIFO first.
     */
    unlink(FIFO_NAME);

    if (mkfifo(FIFO_NAME, 0666) == -1) {

        perror("mkfifo");
        return 1;
    }

    printf("[System] Named FIFO created: %s\n\n",
           FIFO_NAME);


    /*
     * Create two processes for demonstration.
     */
    pid = fork();

    if (pid < 0) {

        perror("fork");
        unlink(FIFO_NAME);
        return 1;
    }


    /*
     * CHILD PROCESS
     *
     * Child reads from the FIFO.
     */
    else if (pid == 0) {

        printf("[Child] Child process started.\n");
        printf("[Child] PID: %d\n", getpid());

        printf("[Child] Opening FIFO for reading...\n");

        int fd = open(FIFO_NAME, O_RDONLY);

        if (fd == -1) {

            perror("open");
            exit(1);
        }

        printf("[Child] FIFO opened for reading.\n");

        ssize_t bytes_read =
            read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {

            perror("read");
            close(fd);
            exit(1);
        }

        buffer[bytes_read] = '\0';

        printf("[Child] Data received:\n");
        printf("        \"%s\"\n", buffer);

        close(fd);

        printf("[Child] FIFO read end closed.\n");

        exit(0);
    }


    /*
     * PARENT PROCESS
     *
     * Parent writes to the FIFO.
     */
    else {

        printf("[Parent] Parent PID: %d\n", getpid());
        printf("[Parent] Child PID : %d\n", pid);

        /*
         * Give the child time to reach open().
         */
        sleep(1);

        printf("[Parent] Opening FIFO for writing...\n");

        int fd = open(FIFO_NAME, O_WRONLY);

        if (fd == -1) {

            perror("open");
            unlink(FIFO_NAME);
            return 1;
        }

        printf("[Parent] FIFO opened for writing.\n");

        ssize_t bytes_written =
            write(fd,
                  message,
                  strlen(message));

        if (bytes_written < 0) {

            perror("write");
            close(fd);
            unlink(FIFO_NAME);
            return 1;
        }

        printf("[Parent] Data sent through FIFO.\n");
        printf("[Parent] Bytes written: %ld\n",
               (long)bytes_written);

        close(fd);

        printf("[Parent] FIFO write end closed.\n");

        /*
         * Wait for child.
         */
        wait(NULL);

        /*
         * Remove the FIFO after communication.
         */
        unlink(FIFO_NAME);

        printf("[Parent] Named FIFO removed.\n");
        printf("[Parent] Child process completed.\n");
    }


    printf("\n========================================\n");
    printf("          NAMED FIFO COMPLETE\n");
    printf("========================================\n");

    return 0;
}