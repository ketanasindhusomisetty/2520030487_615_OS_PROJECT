//Step 3: System Calls & Kernel Services

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {

    printf("========================================\n");
    printf("       CO-1: SYSTEM CALL SERVICES\n");
    printf("========================================\n\n");

    /* ------------------------------------
       1. Process-related system call
       ------------------------------------ */

    pid_t pid = getpid();

    printf("[1] Process Service\n");
    printf("    getpid() -> Current PID: %d\n\n", pid);


    /* ------------------------------------
       2. File-related system calls
       ------------------------------------ */

    printf("[2] File Service\n");

    int fd = open("co1_output.txt",
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("    open() -> File opened successfully\n");

    const char message[] =
        "Hello from Linux system calls!\n";

    ssize_t bytes_written =
        write(fd, message, sizeof(message) - 1);

    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("    write() -> %ld bytes written\n",
           (long)bytes_written);

    close(fd);

    printf("    close() -> File closed successfully\n\n");


    /* ------------------------------------
       3. Directory / Working Environment
       ------------------------------------ */

    printf("[3] Process Environment\n");

    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("    Current Working Directory:\n");
        printf("    %s\n", cwd);
    } else {
        perror("getcwd");
    }


    /* ------------------------------------
       Completion
       ------------------------------------ */

    printf("\n========================================\n");
    printf("    SYSTEM CALL DEMONSTRATION COMPLETE\n");
    printf("========================================\n");

    return 0;
}