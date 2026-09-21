//Step 4 — Shell & Command Execution

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    printf("========================================\n");
    printf("      CO-1: SHELL & COMMAND EXECUTION\n");
    printf("========================================\n\n");

    printf("[1] Current C Program\n");
    printf("    PID: %d\n\n", getpid());

    printf("[2] Executing Linux command: ls -l\n\n");

    /*
       system() asks the shell to execute
       the given command.
    */
    int status = system("ls -l");

    if (status == -1) {
        perror("system");
        return 1;
    }

    printf("\n[3] Linux command execution completed.\n");

    printf("\nCommand Execution Journey:\n");
    printf("----------------------------------------\n");
    printf("C Program\n");
    printf("   |\n");
    printf("   v\n");
    printf("system()\n");
    printf("   |\n");
    printf("   v\n");
    printf("Shell\n");
    printf("   |\n");
    printf("   v\n");
    printf("Linux Command (ls -l)\n");
    printf("   |\n");
    printf("   v\n");
    printf("Kernel Services\n");
    printf("----------------------------------------\n");

    printf("\n========================================\n");
    printf("          CO-1 STEP 4 COMPLETE\n");
    printf("========================================\n");

    return 0;
}