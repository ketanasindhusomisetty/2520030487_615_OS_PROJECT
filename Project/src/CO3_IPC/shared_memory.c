//Step 4: Shared Memory

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define SHARED_MEMORY_SIZE 256

int main() {

    int shmid;
    pid_t pid;

    char *shared_data;

    printf("========================================\n");
    printf("       CO-3: SHARED MEMORY IPC\n");
    printf("========================================\n\n");


    /*
     * Create a shared memory segment.
     *
     * IPC_PRIVATE creates a new private
     * shared memory segment.
     */
    shmid = shmget(
        IPC_PRIVATE,
        SHARED_MEMORY_SIZE,
        IPC_CREAT | 0666
    );

    if (shmid == -1) {

        perror("shmget");
        return 1;
    }

    printf("[System] Shared memory created.\n");
    printf("[System] Shared Memory ID: %d\n", shmid);


    /*
     * Attach shared memory to the current
     * process address space.
     */
    shared_data = (char *)shmat(
        shmid,
        NULL,
        0
    );

    if (shared_data == (char *)-1) {

        perror("shmat");

        shmctl(
            shmid,
            IPC_RMID,
            NULL
        );

        return 1;
    }

    printf("[System] Shared memory attached.\n");


    /*
     * Create child process.
     */
    pid = fork();

    if (pid < 0) {

        perror("fork");

        shmdt(shared_data);

        shmctl(
            shmid,
            IPC_RMID,
            NULL
        );

        return 1;
    }


    /*
     * CHILD PROCESS
     */
    else if (pid == 0) {

        printf("\n[Child] Child process started.\n");
        printf("[Child] PID: %d\n", getpid());

        /*
         * Give the parent time to write
         * into shared memory.
         */
        sleep(2);

        printf("[Child] Reading shared memory...\n");

        printf("[Child] Data received:\n");
        printf("        \"%s\"\n", shared_data);

        /*
         * Detach shared memory.
         */
        if (shmdt(shared_data) == -1) {

            perror("shmdt");
            exit(1);
        }

        printf("[Child] Shared memory detached.\n");

        exit(0);
    }


    /*
     * PARENT PROCESS
     */
    else {

        printf("\n[Parent] Parent process started.\n");
        printf("[Parent] PID: %d\n", getpid());
        printf("[Parent] Child PID: %d\n", pid);


        /*
         * Write data into shared memory.
         */
        strcpy(
            shared_data,
            "Priority Scheduling data stored in Shared Memory."
        );

        printf("[Parent] Data written to shared memory.\n");


        /*
         * Wait for child.
         */
        wait(NULL);

        printf("[Parent] Child process completed.\n");


        /*
         * Detach shared memory.
         */
        if (shmdt(shared_data) == -1) {

            perror("shmdt");
            return 1;
        }

        printf("[Parent] Shared memory detached.\n");


        /*
         * Remove shared memory segment.
         */
        if (shmctl(
                shmid,
                IPC_RMID,
                NULL
            ) == -1) {

            perror("shmctl");
            return 1;
        }

        printf("[Parent] Shared memory removed.\n");
    }


    printf("\n========================================\n");
    printf("       SHARED MEMORY COMPLETE\n");
    printf("========================================\n");

    return 0;
}