//Step 1: Basic Linux System-Call Demo 

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>

int main() {

    struct utsname system_info;

    printf("========================================\n");
    printf("       CO-1: LINUX SYSTEM CALLS\n");
    printf("========================================\n\n");

    // 1. Get Process ID
    printf("[1] Process ID (PID): %d\n", getpid());

    // 2. Get Parent Process ID
    printf("[2] Parent Process ID (PPID): %d\n", getppid());

    // 3. Get User ID
    printf("[3] User ID (UID): %d\n", getuid());

    // 4. Get Linux system information
    if (uname(&system_info) == 0) {
        printf("\n[4] Linux System Information\n");
        printf("    System   : %s\n", system_info.sysname);
        printf("    Node     : %s\n", system_info.nodename);
        printf("    Release  : %s\n", system_info.release);
        printf("    Version  : %s\n", system_info.version);
        printf("    Machine  : %s\n", system_info.machine);
    } else {
        perror("uname");
        return 1;
    }

    // 5. Demonstrate write() system call
    const char message[] =
        "\n[5] write() system call executed successfully.\n";

    write(STDOUT_FILENO, message, sizeof(message) - 1);

    printf("\n========================================\n");
    printf("        CO-1 DEMO COMPLETED\n");
    printf("========================================\n");

    return 0;
}