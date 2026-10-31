#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <pthread.h>
#include <semaphore.h>


#define MAX_PROCESSES 20
#define MAX_GANTT 200

int global_memory_demo = 100;

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int remaining_time;

    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;

    int started;
    int completed;
} Process;

typedef struct {
    int pid;
    int start_time;
    int end_time;
} GanttEntry;

/* =========================================================
   RESET SCHEDULING INFORMATION
   ========================================================= */

void reset_processes(Process p[], int n) {

    for (int i = 0; i < n; i++) {

        p[i].remaining_time = p[i].burst_time;

        p[i].completion_time = 0;
        p[i].turnaround_time = 0;
        p[i].waiting_time = 0;
        p[i].response_time = -1;

        p[i].started = 0;
        p[i].completed = 0;
    }
}


/* =========================================================
   ENTER PROCESS DETAILS
   ========================================================= */

/* =========================================================
   INPUT VALIDATION HELPERS
   ========================================================= */

void clear_input_buffer() {

    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF);
}


int read_integer(const char *message) {

    int value;

    while (1) {

        printf("%s", message);

        if (scanf("%d", &value) == 1) {

            clear_input_buffer();

            return value;
        }

        printf("Invalid input. Please enter an integer.\n");

        clear_input_buffer();
    }
}


int pid_exists(Process p[], int count, int pid) {

    for (int i = 0; i < count; i++) {

        if (p[i].pid == pid) {
            return 1;
        }
    }

    return 0;
}


/* =========================================================
   ENTER PROCESS DETAILS
   ========================================================= */

void input_processes(Process p[], int *n) {

    printf("\n================================================\n");
    printf("              PROCESS INPUT\n");
    printf("================================================\n");

    /* Number of processes */

    while (1) {

        *n = read_integer(
            "\nEnter number of processes (1-20): "
        );

        if (*n >= 1 && *n <= MAX_PROCESSES) {
            break;
        }

        printf(
            "Invalid number of processes.\n"
            "Please enter a value between 1 and %d.\n",
            MAX_PROCESSES
        );
    }


    /* Process details */

    for (int i = 0; i < *n; i++) {

        printf("\n--------------------------------------------\n");
        printf("              PROCESS %d\n", i + 1);
        printf("--------------------------------------------\n");


        /* PID */

        while (1) {

            p[i].pid = read_integer("PID          : ");

            if (p[i].pid <= 0) {

                printf(
                    "PID must be greater than 0.\n"
                );

                continue;
            }

            if (pid_exists(p, i, p[i].pid)) {

                printf(
                    "PID already exists. "
                    "Please enter a unique PID.\n"
                );

                continue;
            }

            break;
        }


        /* Arrival Time */

        while (1) {

            p[i].arrival_time =
                read_integer("Arrival Time : ");

            if (p[i].arrival_time < 0) {

                printf(
                    "Arrival Time cannot be negative.\n"
                );

                continue;
            }

            break;
        }


        /* Burst Time */

        while (1) {

            p[i].burst_time =
                read_integer("Burst Time   : ");

            if (p[i].burst_time <= 0) {

                printf(
                    "Burst Time must be greater than 0.\n"
                );

                continue;
            }

            break;
        }


        /* Priority */

        while (1) {

            p[i].priority =
                read_integer("Priority     : ");

            if (p[i].priority <= 0) {

                printf(
                    "Priority must be greater than 0.\n"
                );

                continue;
            }

            break;
        }
    }


    /* Initialize scheduling information */

    reset_processes(p, *n);


    printf("\n================================================\n");
    printf("       PROCESS DETAILS STORED SUCCESSFULLY\n");
    printf("================================================\n");
}


/* =========================================================
   DISPLAY PROCESS DETAILS
   ========================================================= */

void display_input(Process p[], int n) {

    if (n == 0) {

        printf("\nNo process information available.\n");
        return;
    }

    printf("\n================================================\n");
    printf("              PROCESS INFORMATION\n");
    printf("================================================\n");

    printf("%-8s %-8s %-8s %-10s\n",
           "PID",
           "AT",
           "BT",
           "Priority");

    printf("------------------------------------------------\n");

    for (int i = 0; i < n; i++) {

        printf("%-8d %-8d %-8d %-10d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].priority);
    }

    printf("================================================\n");

    printf("\nPriority Rule:\n");
    printf("Smaller priority number = Higher priority\n");
}


/* =========================================================
   SELECT HIGHEST PRIORITY PROCESS
   ========================================================= */

int select_process(Process p[], int n, int current_time) {

    int selected = -1;

    for (int i = 0; i < n; i++) {

        if (p[i].arrival_time <= current_time &&
            !p[i].completed &&
            p[i].remaining_time > 0) {

            if (selected == -1) {

                selected = i;

            }

            else if (p[i].priority <
                     p[selected].priority) {

                selected = i;

            }

            else if (p[i].priority ==
                     p[selected].priority) {

                /*
                 * Tie breaker:
                 * Earlier arrival time first.
                 */

                if (p[i].arrival_time <
                    p[selected].arrival_time) {

                    selected = i;
                }

                /*
                 * If arrival time is also the same,
                 * smaller PID is selected.
                 */

                else if (p[i].arrival_time ==
                         p[selected].arrival_time &&
                         p[i].pid <
                         p[selected].pid) {

                    selected = i;
                }
            }
        }
    }

    return selected;
}


/* =========================================================
   FIND NEXT ARRIVAL TIME
   ========================================================= */

int find_next_arrival(Process p[], int n) {

    int next_arrival = -1;

    for (int i = 0; i < n; i++) {

        if (!p[i].completed) {

            if (next_arrival == -1 ||
                p[i].arrival_time < next_arrival) {

                next_arrival = p[i].arrival_time;
            }
        }
    }

    return next_arrival;
}


/* =========================================================
   ADD GANTT ENTRY
   ========================================================= */

void add_gantt_entry(
    GanttEntry gantt[],
    int *count,
    int pid,
    int start,
    int end
) {

    if (start == end)
        return;

    /*
     * Merge consecutive execution
     * of the same process.
     */

    if (*count > 0 &&
        gantt[*count - 1].pid == pid &&
        gantt[*count - 1].end_time == start) {

        gantt[*count - 1].end_time = end;

        return;
    }

    if (*count < MAX_GANTT) {

        gantt[*count].pid = pid;
        gantt[*count].start_time = start;
        gantt[*count].end_time = end;

        (*count)++;
    }
}


/* =========================================================
   DISPLAY GANTT CHART
   ========================================================= */

void display_gantt(GanttEntry gantt[], int count) {

    printf("\n================================================\n");
    printf("                 GANTT CHART\n");
    printf("================================================\n\n");

    if (count == 0) {

        printf("No Gantt Chart available.\n");
        return;
    }

    printf("|");

    for (int i = 0; i < count; i++) {

        if (gantt[i].pid == -1)
            printf(" IDLE |");
        else
            printf(" P%d |", gantt[i].pid);
    }

    printf("\n");

    printf("%d", gantt[0].start_time);

    for (int i = 0; i < count; i++) {

        printf("     %d",
               gantt[i].end_time);
    }

    printf("\n");
}


/* =========================================================
   DISPLAY SCHEDULING RESULTS
   ========================================================= */

void display_results(Process p[], int n) {

    double total_waiting = 0;
    double total_turnaround = 0;
    double total_response = 0;

    printf("\n====================================================================\n");
    printf("                    SCHEDULING RESULTS\n");
    printf("====================================================================\n");

    printf("%-6s %-6s %-6s %-8s %-6s %-6s %-6s %-6s\n",
           "PID",
           "AT",
           "BT",
           "Priority",
           "CT",
           "TAT",
           "WT",
           "RT");

    printf("--------------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {

        printf("%-6d %-6d %-6d %-8d %-6d %-6d %-6d %-6d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].priority,
               p[i].completion_time,
               p[i].turnaround_time,
               p[i].waiting_time,
               p[i].response_time);

        total_waiting += p[i].waiting_time;
        total_turnaround += p[i].turnaround_time;
        total_response += p[i].response_time;
    }

    printf("====================================================================\n");

    printf("\nAverage Waiting Time    : %.2f\n",
           total_waiting / n);

    printf("Average Turnaround Time : %.2f\n",
           total_turnaround / n);

    printf("Average Response Time   : %.2f\n",
           total_response / n);
}


/* =========================================================
   NON-PREEMPTIVE PRIORITY SCHEDULING
   ========================================================= */

void non_preemptive_priority(Process p[], int n) {

    if (n == 0) {

        printf("\nPlease enter process details first.\n");
        return;
    }

    reset_processes(p, n);

    GanttEntry gantt[MAX_GANTT];

    int current_time = 0;
    int completed = 0;
    int gantt_count = 0;

    printf("\n================================================\n");
    printf("       NON-PREEMPTIVE PRIORITY SCHEDULING\n");
    printf("================================================\n");

    printf("\nPriority Rule:\n");
    printf("Smaller priority number = Higher priority\n");

    while (completed < n) {

        int selected =
            select_process(
                p,
                n,
                current_time
            );

        /*
         * No process is currently available.
         */

        if (selected == -1) {

            int next_arrival =
                find_next_arrival(p, n);

            if (next_arrival > current_time) {

                add_gantt_entry(
                    gantt,
                    &gantt_count,
                    -1,
                    current_time,
                    next_arrival
                );

                current_time = next_arrival;
            }

            continue;
        }

        /*
         * First CPU execution.
         */

        p[selected].started = 1;

        p[selected].response_time =
            current_time -
            p[selected].arrival_time;

        /*
         * Start execution.
         */

        int start_time = current_time;

        /*
         * NON-PREEMPTIVE:
         * Process executes completely.
         */

        current_time +=
            p[selected].burst_time;

        /*
         * Add Gantt entry.
         */

        add_gantt_entry(
            gantt,
            &gantt_count,
            p[selected].pid,
            start_time,
            current_time
        );

        /*
         * Process completed.
         */

        p[selected].remaining_time = 0;
        p[selected].completed = 1;

        completed++;

        /*
         * Calculate metrics.
         */

        p[selected].completion_time =
            current_time;

        p[selected].turnaround_time =
            p[selected].completion_time -
            p[selected].arrival_time;

        p[selected].waiting_time =
            p[selected].turnaround_time -
            p[selected].burst_time;
    }

    display_results(p, n);

    display_gantt(gantt, gantt_count);

    printf("\n================================================\n");
    printf("     NON-PREEMPTIVE SCHEDULING COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   PREEMPTIVE PRIORITY SCHEDULING
   ========================================================= */

void preemptive_priority(Process p[], int n) {

    if (n == 0) {

        printf("\nPlease enter process details first.\n");
        return;
    }

    reset_processes(p, n);

    GanttEntry gantt[MAX_GANTT];

    int current_time = 0;
    int completed = 0;
    int gantt_count = 0;

    printf("\n================================================\n");
    printf("         PREEMPTIVE PRIORITY SCHEDULING\n");
    printf("================================================\n");

    printf("\nPriority Rule:\n");
    printf("Smaller priority number = Higher priority\n");

    while (completed < n) {

        int selected =
            select_process(
                p,
                n,
                current_time
            );

        /*
         * CPU IDLE
         */

        if (selected == -1) {

            int next_arrival =
                find_next_arrival(p, n);

            if (next_arrival > current_time) {

                add_gantt_entry(
                    gantt,
                    &gantt_count,
                    -1,
                    current_time,
                    next_arrival
                );

                current_time =
                    next_arrival;
            }

            continue;
        }

        /*
         * First time process gets CPU.
         */

        if (!p[selected].started) {

            p[selected].started = 1;

            p[selected].response_time =
                current_time -
                p[selected].arrival_time;
        }

        /*
         * Execute for ONE time unit.
         *
         * This allows preemption.
         */

        int start_time = current_time;

        current_time++;

        p[selected].remaining_time--;

        /*
         * Add execution to Gantt Chart.
         */

        add_gantt_entry(
            gantt,
            &gantt_count,
            p[selected].pid,
            start_time,
            current_time
        );

        /*
         * Process completed.
         */

        if (p[selected].remaining_time == 0) {

            p[selected].completed = 1;

            completed++;

            p[selected].completion_time =
                current_time;

            p[selected].turnaround_time =
                p[selected].completion_time -
                p[selected].arrival_time;

            p[selected].waiting_time =
                p[selected].turnaround_time -
                p[selected].burst_time;
        }
    }

    display_results(p, n);

    display_gantt(gantt, gantt_count);

    printf("\n================================================\n");
    printf("       PREEMPTIVE SCHEDULING COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   PROCESS MANAGEMENT DEMO
   Demonstrates fork(), exec() and wait()
   ========================================================= */

void process_management_demo() {

    pid_t pid;
    int status;

    printf("\n================================================\n");
    printf("             PROCESS MANAGEMENT DEMO\n");
    printf("================================================\n");

    printf("\nParent Process ID : %d\n", getpid());

    printf("\nCreating child process using fork()...\n");

    pid = fork();

    /*
     * fork() failed
     */
    if (pid < 0) {

        perror("fork failed");
        return;
    }


    /*
     * Child process
     */
    if (pid == 0) {

        printf("\n--- CHILD PROCESS ---\n");

        printf("Child Process ID  : %d\n", getpid());
        printf("Parent Process ID : %d\n", getppid());

        printf("\nChild is executing a new program using exec()...\n");

        /*
         * Replace child process with ls -l
         */
        execl(
            "/bin/ls",
            "ls",
            "-l",
            NULL
        );

        /*
         * This executes only if execl() fails.
         */
        perror("exec failed");

        exit(1);
    }


    /*
     * Parent process
     */
    else {

        printf("\n--- PARENT PROCESS ---\n");

        printf("Parent Process ID : %d\n", getpid());
        printf("Child Process ID  : %d\n", pid);

        printf("\nParent is waiting for child using wait()...\n");

        wait(&status);

        if (WIFEXITED(status)) {

            printf(
                "\nChild process terminated normally.\n"
            );

            printf(
                "Child Exit Status : %d\n",
                WEXITSTATUS(status)
            );
        }

        printf("\nParent process continues execution.\n");
    }

    printf("\n================================================\n");
    printf("        PROCESS MANAGEMENT DEMO COMPLETE\n");
    printf("================================================\n");
}

/* =========================================================
   ANONYMOUS PIPE DEMO
   Demonstrates IPC using pipe(), write() and read()
   ========================================================= */

void anonymous_pipe_demo() {

    int pipe_fd[2];

    pid_t pid;

    char message[] =
        "Hello from the Priority Scheduling parent process!";

    char buffer[100];

    printf("\n================================================\n");
    printf("             ANONYMOUS PIPE DEMO\n");
    printf("================================================\n");


    /*
     * Create the pipe.
     *
     * pipe_fd[0] -> read end
     * pipe_fd[1] -> write end
     */

    if (pipe(pipe_fd) == -1) {

        perror("pipe failed");
        return;
    }


    /*
     * Create child process.
     */

    pid = fork();

    if (pid < 0) {

        perror("fork failed");

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        return;
    }


    /*
     * CHILD PROCESS
     */

    if (pid == 0) {

        /*
         * Child only reads.
         * Close unused write end.
         */

        close(pipe_fd[1]);

        printf("\n--- CHILD PROCESS ---\n");

        printf("Child PID : %d\n", getpid());

        /*
         * Read message from parent.
         */

        ssize_t bytes_read =
            read(
                pipe_fd[0],
                buffer,
                sizeof(buffer) - 1
            );

        if (bytes_read < 0) {

            perror("read failed");

            close(pipe_fd[0]);

            exit(1);
        }

        buffer[bytes_read] = '\0';

        printf(
            "Message received: %s\n",
            buffer
        );

        close(pipe_fd[0]);

        exit(0);
    }


    /*
     * PARENT PROCESS
     */

    else {

        /*
         * Parent only writes.
         * Close unused read end.
         */

        close(pipe_fd[0]);

        printf("\n--- PARENT PROCESS ---\n");

        printf("Parent PID : %d\n", getpid());

        printf(
            "Sending message through pipe...\n"
        );

        /*
         * Write message to pipe.
         */

        ssize_t bytes_written =
            write(
                pipe_fd[1],
                message,
                sizeof(message)
            );

        if (bytes_written < 0) {

            perror("write failed");

            close(pipe_fd[1]);

            wait(NULL);

            return;
        }

        printf(
            "Message sent successfully.\n"
        );

        close(pipe_fd[1]);

        /*
         * Wait for child.
         */

        wait(NULL);

        printf(
            "Child process completed.\n"
        );
    }


    printf("\n================================================\n");
    printf("          ANONYMOUS PIPE DEMO COMPLETE\n");
    printf("================================================\n");
}

void handle_sigusr1(int signal_number) {

    const char message[] =
        "Child received SIGUSR1 successfully!\n";

    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

void signal_demo() {

    pid_t pid;

    printf("\n================================================\n");
    printf("                SIGNAL DEMO\n");
    printf("================================================\n");

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {

        printf("\n--- CHILD PROCESS ---\n");
        printf("Child PID : %d\n", getpid());

        signal(SIGUSR1, handle_sigusr1);

        printf("Child is waiting for SIGUSR1...\n");

        pause();

        printf("Child process resumed after receiving signal.\n");
        printf("Child process completed.\n");

        exit(0);

    } else {

        printf("\n--- PARENT PROCESS ---\n");
        printf("Parent PID : %d\n", getpid());

        sleep(1);

        printf("Sending SIGUSR1 to child...\n");

        if (kill(pid, SIGUSR1) == -1) {
            perror("kill failed");
            wait(NULL);
            return;
        }

        printf("SIGUSR1 sent successfully.\n");

        wait(NULL);

        printf("Child process completed.\n");
    }

    printf("\n================================================\n");
    printf("              SIGNAL DEMO COMPLETE\n");
    printf("================================================\n");
}

void shared_memory_demo() {

    int shmid;
    pid_t pid;

    char *shared_data;

    const int SHM_SIZE = 256;

    printf("\n================================================\n");
    printf("             SHARED MEMORY DEMO\n");
    printf("================================================\n");

    /* Create shared memory segment */
    shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);

    if (shmid == -1) {
        perror("shmget failed");
        return;
    }

    /* Attach shared memory */
    shared_data = (char *)shmat(shmid, NULL, 0);

    if (shared_data == (char *)-1) {
        perror("shmat failed");
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    printf("\nShared memory segment created successfully.\n");
    printf("Shared Memory ID : %d\n", shmid);

    /* Parent writes data before creating child */
    snprintf(
        shared_data,
        SHM_SIZE,
        "Priority Scheduling data shared successfully!"
    );

    printf("\n--- PARENT PROCESS ---\n");
    printf("Parent PID : %d\n", getpid());
    printf("Writing data to shared memory...\n");
    printf("Data written successfully.\n");

    pid = fork();

    if (pid < 0) {

        perror("fork failed");
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
        return;

    } else if (pid == 0) {

        printf("\n--- CHILD PROCESS ---\n");
        printf("Child PID : %d\n", getpid());

        /* Child attaches to the same shared memory */
        char *child_data =
            (char *)shmat(shmid, NULL, 0);

        if (child_data == (char *)-1) {
            perror("child shmat failed");
            exit(1);
        }

        printf("Reading data from shared memory...\n");
        printf("Message received: %s\n", child_data);

        shmdt(child_data);

        printf("Child process completed.\n");

        exit(0);

    } else {

        wait(NULL);

        printf("\nChild process completed.\n");
    }

    /* Detach shared memory */
    shmdt(shared_data);

    /* Remove shared memory segment */
    shmctl(shmid, IPC_RMID, NULL);

    printf("\nShared memory segment removed successfully.\n");

    printf("\n================================================\n");
    printf("          SHARED MEMORY DEMO COMPLETE\n");
    printf("================================================\n");
}

void memory_management_demo() {

    int stack_variable = 10;
    int *heap_variable;

    printf("\n================================================\n");
    printf("           MEMORY MANAGEMENT DEMO\n");
    printf("================================================\n");

    printf("\n--- LINUX PROCESS ADDRESS SPACE ---\n");

    printf("Text / Code address : %p\n",
           (void *)memory_management_demo);

    printf("Global variable     : %p\n",
           (void *)&global_memory_demo);

    printf("Stack variable      : %p\n",
           (void *)&stack_variable);

    /* Dynamic memory allocation */
    heap_variable = (int *)malloc(sizeof(int));

    if (heap_variable == NULL) {
        printf("\nMemory allocation failed.\n");
        return;
    }

    *heap_variable = 50;

    printf("Heap memory address  : %p\n",
           (void *)heap_variable);

    printf("\n--- DYNAMIC MEMORY ALLOCATION ---\n");

    printf("Allocated memory successfully.\n");
    printf("Value stored in heap memory : %d\n",
           *heap_variable);

    /* Release allocated memory */
    free(heap_variable);

    printf("Heap memory released using free().\n");

    printf("\n--- MEMORY AREAS OBSERVED ---\n");
    printf("1. Text / Code  -> Program instructions\n");
    printf("2. Global/Data  -> Global variables\n");
    printf("3. Heap         -> Dynamically allocated memory\n");
    printf("4. Stack        -> Local variables and function calls\n");

    printf("\n================================================\n");
    printf("        MEMORY MANAGEMENT DEMO COMPLETE\n");
    printf("================================================\n");
}

void copy_on_write_demo() {

    int *shared_value;
    pid_t pid;

    shared_value = (int *)malloc(sizeof(int));

    if (shared_value == NULL) {
        printf("\nMemory allocation failed.\n");
        return;
    }

    *shared_value = 100;

    printf("\n================================================\n");
    printf("             COPY-ON-WRITE DEMO\n");
    printf("================================================\n");

    printf("\nMemory allocated before fork().\n");
    printf("Initial value : %d\n", *shared_value);
    printf("Memory address: %p\n", (void *)shared_value);

    pid = fork();

    if (pid < 0) {

        perror("fork failed");
        free(shared_value);
        return;

    } else if (pid == 0) {

        printf("\n--- CHILD PROCESS ---\n");
        printf("Child PID : %d\n", getpid());

        printf("Value before child writes : %d\n",
               *shared_value);

        printf("Child modifies the value...\n");

        *shared_value = 200;

        printf("Value after child writes  : %d\n",
               *shared_value);

        printf("Child memory address      : %p\n",
               (void *)shared_value);

        printf("\nChild has its own modified copy after writing.\n");

        free(shared_value);
        exit(0);

    } else {

        wait(NULL);

        printf("\n--- PARENT PROCESS ---\n");
        printf("Parent PID : %d\n", getpid());

        printf("Parent value after child modification : %d\n",
               *shared_value);

        printf("Parent memory address                 : %p\n",
               (void *)shared_value);

        printf("\nParent retains its original value.\n");
        printf("This demonstrates the effect of Copy-on-Write.\n");

        free(shared_value);
    }

    printf("\n================================================\n");
    printf("          COPY-ON-WRITE DEMO COMPLETE\n");
    printf("================================================\n");
}

void virtual_memory_demo() {

    int page_size = 256;
    int virtual_address;
    int page_number;
    int offset;
    int frame_number;
    int physical_address;

    /* Simple simulated page table */
    int page_table[4] = {3, 1, 4, 2};

    printf("\n================================================\n");
    printf("       VIRTUAL MEMORY & PAGE TABLE DEMO\n");
    printf("================================================\n");

    printf("\nThis is a simulation of address translation.\n");

    printf("\nEnter a virtual address (0 - 1023): ");
    scanf("%d", &virtual_address);

    if (virtual_address < 0 || virtual_address > 1023) {
        printf("Invalid virtual address.\n");
        clear_input_buffer();
        return;
    }

    /*
     * Divide virtual address into:
     * page number + offset
     */
    page_number = virtual_address / page_size;
    offset = virtual_address % page_size;

    printf("\n--- ADDRESS TRANSLATION ---\n");
    printf("Virtual Address : %d\n", virtual_address);
    printf("Page Size       : %d bytes\n", page_size);
    printf("Page Number     : %d\n", page_number);
    printf("Offset          : %d\n", offset);

    printf("\n--- PAGE TABLE ---\n");

    for (int i = 0; i < 4; i++) {
        printf("Page %d -> Frame %d\n",
               i, page_table[i]);
    }

    frame_number = page_table[page_number];

    physical_address =
        (frame_number * page_size) + offset;

    printf("\n--- TRANSLATION RESULT ---\n");
    printf("Page Number     : %d\n", page_number);
    printf("Frame Number    : %d\n", frame_number);
    printf("Offset          : %d\n", offset);
    printf("Physical Address: %d\n", physical_address);

    printf("\n================================================\n");
    printf("       VIRTUAL MEMORY DEMO COMPLETE\n");
    printf("================================================\n");
}

void page_fault_demo() {

    int page_table[4] = {1, -1, 3, -1};
    int page_present[4] = {1, 0, 1, 0};

    int requested_page;
    int free_frame = 4;

    printf("\n================================================\n");
    printf("       PAGE FAULT & DEMAND PAGING DEMO\n");
    printf("================================================\n");

    printf("\nPage Table Status:\n");

    for (int i = 0; i < 4; i++) {

        if (page_present[i]) {
            printf("Page %d -> Frame %d [PRESENT]\n",
                   i, page_table[i]);
        } else {
            printf("Page %d -> Not in memory [NOT PRESENT]\n",
                   i);
        }
    }

    printf("\nEnter a page number to access (0 - 3): ");
    scanf("%d", &requested_page);

    if (requested_page < 0 || requested_page > 3) {

        printf("Invalid page number.\n");
        clear_input_buffer();
        return;
    }

    printf("\nRequesting Page %d...\n", requested_page);

    if (page_present[requested_page]) {

        printf("Page %d is already in physical memory.\n",
               requested_page);

        printf("No page fault occurred.\n");

    } else {

        printf("\nPAGE FAULT OCCURRED!\n");
        printf("Page %d is not currently in physical memory.\n",
               requested_page);

        printf("Demand paging loads the required page into memory.\n");

        page_table[requested_page] = free_frame;
        page_present[requested_page] = 1;

        printf("Page %d loaded into Frame %d.\n",
               requested_page, free_frame);

        printf("Page table updated successfully.\n");
    }

    printf("\n--- UPDATED PAGE TABLE ---\n");

    for (int i = 0; i < 4; i++) {

        if (page_present[i]) {
            printf("Page %d -> Frame %d [PRESENT]\n",
                   i, page_table[i]);
        } else {
            printf("Page %d -> Not in memory [NOT PRESENT]\n",
                   i);
        }
    }

    printf("\n================================================\n");
    printf("       PAGE FAULT DEMO COMPLETE\n");
    printf("================================================\n");
}

void memory_errors_demo() {

    int *ptr;
    int *leak_ptr;

    printf("\n================================================\n");
    printf("             MEMORY ERRORS DEMO\n");
    printf("================================================\n");

    /* Normal allocation */
    ptr = (int *)malloc(sizeof(int));

    if (ptr == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    *ptr = 100;

    printf("\n--- VALID MEMORY ACCESS ---\n");
    printf("Allocated memory successfully.\n");
    printf("Stored value : %d\n", *ptr);

    free(ptr);
    ptr = NULL;

    printf("Memory released successfully.\n");

    /* Demonstrate a potential memory leak */
    printf("\n--- MEMORY LEAK ---\n");

    leak_ptr = (int *)malloc(5 * sizeof(int));

    if (leak_ptr == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    printf("Memory allocated dynamically.\n");
    printf("If this memory is not released using free(),\n");
    printf("it can result in a memory leak.\n");

    free(leak_ptr);

    printf("Allocated memory released successfully.\n");

    /* Explain invalid access safely */
    printf("\n--- INVALID MEMORY ACCESS ---\n");
    printf("Accessing memory after free() can cause\n");
    printf("undefined behavior such as a segmentation fault.\n");

    printf("\n--- DOUBLE FREE ---\n");
    printf("Calling free() twice on the same pointer is unsafe.\n");
    printf("It can cause undefined behavior or program termination.\n");

    printf("\n--- OPERATING SYSTEM RESPONSE ---\n");
    printf("Invalid memory access may result in a segmentation fault.\n");
    printf("The operating system protects memory belonging to other processes.\n");

    printf("\n================================================\n");
    printf("           MEMORY ERRORS DEMO COMPLETE\n");
    printf("================================================\n");
}

void memory_debugging_demo() {

    char command[100];

    printf("\n================================================\n");
    printf("       MEMORY DEBUGGING & ANALYSIS DEMO\n");
    printf("================================================\n");

    printf("\n--- CURRENT PROCESS MEMORY INFORMATION ---\n");

    printf("Process PID : %d\n", getpid());

    snprintf(command, sizeof(command),
             "grep -E 'VmSize|VmRSS|VmData|VmStk|VmExe' /proc/%d/status",
             getpid());

    printf("\nMemory information from /proc:\n\n");

    system(command);

    printf("\n--- SYSTEM MEMORY INFORMATION ---\n");

    system("free -h");

    printf("\n--- PROCESS MEMORY SUMMARY ---\n");

    snprintf(command, sizeof(command),
             "ps -o pid,ppid,rss,vsz,comm -p %d",
             getpid());

    system(command);

    printf("\n--- MEMORY DEBUGGING TOOLS ---\n");
    printf("1. /proc/<PID>/status -> Process memory information\n");
    printf("2. free               -> System memory usage\n");
    printf("3. ps                 -> Process memory summary\n");
    printf("4. GCC warnings       -> Detect possible coding problems\n");
    printf("5. Valgrind           -> Detect memory leaks and invalid access\n");

    printf("\n================================================\n");
    printf("      MEMORY DEBUGGING DEMO COMPLETE\n");
    printf("================================================\n");
}

void named_fifo_demo() {

    const char *fifo_path = "/tmp/priority_scheduler_fifo";
    pid_t pid;

    char message[] =
        "Hello through Named FIFO from Priority Scheduling!";
    char buffer[100];

    printf("\n================================================\n");
    printf("              NAMED FIFO DEMO\n");
    printf("================================================\n");

    /* Create the FIFO */
    if (mkfifo(fifo_path, 0666) == -1) {

        /* FIFO may already exist from an earlier run */
        if (access(fifo_path, F_OK) != 0) {
            perror("mkfifo failed");
            return;
        }
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        unlink(fifo_path);
        return;
    }

    if (pid == 0) {

        printf("\n--- CHILD PROCESS ---\n");
        printf("Child PID : %d\n", getpid());
        printf("Waiting for message through FIFO...\n");

        int fd = open(fifo_path, O_RDONLY);

        if (fd == -1) {
            perror("FIFO open for reading failed");
            exit(1);
        }

        ssize_t bytes_read =
            read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {
            perror("FIFO read failed");
            close(fd);
            exit(1);
        }

        buffer[bytes_read] = '\0';

        printf("Message received: %s\n", buffer);

        close(fd);
        exit(0);

    } else {

        printf("\n--- PARENT PROCESS ---\n");
        printf("Parent PID : %d\n", getpid());

        sleep(1);

        int fd = open(fifo_path, O_WRONLY);

        if (fd == -1) {
            perror("FIFO open for writing failed");
            wait(NULL);
            unlink(fifo_path);
            return;
        }

        printf("Sending message through FIFO...\n");

        ssize_t bytes_written =
            write(fd, message, sizeof(message));

        if (bytes_written < 0) {
            perror("FIFO write failed");
            close(fd);
            wait(NULL);
            unlink(fifo_path);
            return;
        }

        printf("Message sent successfully.\n");

        close(fd);

        wait(NULL);

        printf("Child process completed.\n");
    }

    /* Remove FIFO after communication */
    unlink(fifo_path);

    printf("\n================================================\n");
    printf("             NAMED FIFO DEMO COMPLETE\n");
    printf("================================================\n");
}

void file_system_demo() {

    const char *filename = "priority_schedule_report.txt";

    int fd;
    int read_fd;

    char report[] =
        "PRIORITY SCHEDULING SYSTEM REPORT\n"
        "=================================\n"
        "File System Demonstration\n"
        "\n"
        "This file was created using Linux file I/O system calls.\n"
        "The application demonstrates file creation, writing,\n"
        "reading and closing using file descriptors.\n";

    char buffer[512];

    ssize_t bytes_written;
    ssize_t bytes_read;

    printf("\n================================================\n");
    printf("             FILE SYSTEM DEMO\n");
    printf("================================================\n");

    /* Open/Create file */
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd == -1) {
        perror("File open failed");
        return;
    }

    printf("\nFile opened successfully.\n");
    printf("File name       : %s\n", filename);
    printf("File descriptor : %d\n", fd);

    /* Write data */
    bytes_written =
        write(fd, report, sizeof(report) - 1);

    if (bytes_written == -1) {
        perror("File write failed");
        close(fd);
        return;
    }

    printf("Data written successfully.\n");
    printf("Bytes written   : %ld\n",
           (long)bytes_written);

    /* Close file */
    close(fd);

    printf("File closed successfully.\n");

    /* Reopen file for reading */
    read_fd = open(filename, O_RDONLY);

    if (read_fd == -1) {
        perror("File reopen failed");
        return;
    }

    printf("\nFile reopened for reading.\n");
    printf("Read file descriptor : %d\n", read_fd);

    bytes_read =
        read(read_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1) {
        perror("File read failed");
        close(read_fd);
        return;
    }

    buffer[bytes_read] = '\0';

    printf("\n--- FILE CONTENT ---\n");
    printf("%s", buffer);

    close(read_fd);

    printf("\nFile closed successfully after reading.");

    printf("\n\n================================================\n");
    printf("          FILE SYSTEM DEMO COMPLETE\n");
    printf("================================================\n");
}

void file_descriptor_demo() {

    const char *filename = "fd_management_demo.txt";

    int fd1;
    int fd2;

    char buffer[100];

    ssize_t bytes_read;

    printf("\n================================================\n");
    printf("       FILE DESCRIPTOR & OPEN FILE DEMO\n");
    printf("================================================\n");

    /* Open a file */
    fd1 = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

    if (fd1 == -1) {
        perror("open failed");
        return;
    }

    printf("\nFile opened successfully.\n");
    printf("Original file descriptor : %d\n", fd1);

    /* Duplicate the file descriptor */
    fd2 = dup(fd1);

    if (fd2 == -1) {
        perror("dup failed");
        close(fd1);
        return;
    }

    printf("Duplicated file descriptor: %d\n", fd2);

    printf("\nBoth descriptors refer to the same open file.\n");

    /* Write using first descriptor */
    if (write(fd1, "Priority ", 9) == -1) {
        perror("write failed");
        close(fd1);
        close(fd2);
        return;
    }

    printf("Data written using fd %d.\n", fd1);

    /* Write using duplicated descriptor */
    if (write(fd2, "Scheduler", 9) == -1) {
        perror("write failed");
        close(fd1);
        close(fd2);
        return;
    }

    printf("Data written using fd %d.\n", fd2);

    /*
     * Both descriptors share the same file offset
     * because they refer to the same open file description.
     */
    if (lseek(fd1, 0, SEEK_SET) == -1) {
        perror("lseek failed");
        close(fd1);
        close(fd2);
        return;
    }

    bytes_read = read(fd2, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1) {
        perror("read failed");
        close(fd1);
        close(fd2);
        return;
    }

    buffer[bytes_read] = '\0';

    printf("\n--- FILE CONTENT ---\n");
    printf("%s\n", buffer);

    printf("\nFile descriptor %d and %d refer to the same\n",
           fd1, fd2);
    printf("open file description and therefore share the\n");
    printf("current file offset.\n");

    close(fd1);
    close(fd2);

    printf("\nBoth file descriptors closed successfully.");

    printf("\n\n================================================\n");
    printf("     FILE DESCRIPTOR DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   BUFFERED VS UNBUFFERED FILE I/O DEMO
   ========================================================= */

void buffered_unbuffered_demo() {

    FILE *file;
    int fd;

    char buffer[200];

    const char *buffered_file = "buffered_demo.txt";
    const char *unbuffered_file = "unbuffered_demo.txt";

    char message[] =
        "Priority Scheduling - Unbuffered I/O demonstration.";

    ssize_t bytes_read;

    printf("\n================================================\n");
    printf("       BUFFERED VS UNBUFFERED FILE I/O\n");
    printf("================================================\n");


    /* ==============================================
       BUFFERED FILE I/O
       ============================================== */

    printf("\n--- BUFFERED FILE I/O ---\n");

    file = fopen(buffered_file, "w");

    if (file == NULL) {

        perror("fopen failed");
        return;
    }

    fprintf(file, "Priority Scheduling System\n");

    fprintf(file, "Buffered File I/O Demonstration\n");

    printf("Data written using fprintf().\n");

    fflush(file);

    printf("Buffer flushed using fflush().\n");

    fclose(file);

    printf("File closed using fclose().\n");


    /* ==============================================
       UNBUFFERED FILE I/O
       ============================================== */

    printf("\n--- UNBUFFERED FILE I/O ---\n");

    fd = open(
        unbuffered_file,
        O_CREAT | O_WRONLY | O_TRUNC,
        0644
    );

    if (fd == -1) {

        perror("open failed");
        return;
    }

    printf("File opened using open().\n");

    if (write(fd, message, sizeof(message) - 1) == -1) {

        perror("write failed");

        close(fd);
        return;
    }

    printf("Data written using write().\n");

    close(fd);

    printf("File closed using close().\n");


    /* ==============================================
       READ UNBUFFERED FILE
       ============================================== */

    fd = open(unbuffered_file, O_RDONLY);

    if (fd == -1) {

        perror("reopen failed");
        return;
    }

    bytes_read = read(
        fd,
        buffer,
        sizeof(buffer) - 1
    );

    if (bytes_read == -1) {

        perror("read failed");

        close(fd);
        return;
    }

    buffer[bytes_read] = '\0';

    printf("\n--- UNBUFFERED FILE CONTENT ---\n");

    printf("%s\n", buffer);

    close(fd);


    /* ==============================================
       COMPARISON
       ============================================== */

    printf("\n--- COMPARISON ---\n");

    printf("Buffered I/O uses C standard library functions.\n");

    printf("Unbuffered I/O uses Linux file I/O system calls.\n");

    printf("\nFiles created successfully:\n");

    printf("1. %s\n", buffered_file);

    printf("2. %s\n", unbuffered_file);


    printf("\n================================================\n");
    printf("       BUFFERED I/O DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   MEMORY-MAPPED FILE I/O DEMO
   ========================================================= */

void memory_mapped_file_demo() {

    int fd;
    char *mapped_data;
    const char *filename = "mmap_demo.txt";

    const char message[] =
        "Priority Scheduling - Memory Mapped File I/O Demo.";

    size_t file_size = sizeof(message);

    printf("\n================================================\n");
    printf("          MEMORY-MAPPED FILE I/O\n");
    printf("================================================\n");


    /* STEP 1: CREATE AND OPEN FILE */

    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open failed");
        return;
    }

    printf("\nFile opened successfully.\n");


    /* STEP 2: SET FILE SIZE */

    if (ftruncate(fd, file_size) == -1) {
        perror("ftruncate failed");
        close(fd);
        return;
    }

    printf("File size allocated successfully.\n");


    /* STEP 3: MAP FILE INTO MEMORY */

    mapped_data = mmap(
        NULL,
        file_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    if (mapped_data == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return;
    }

    printf("File mapped into virtual memory.\n");


    /* STEP 4: WRITE THROUGH MEMORY */

    strcpy(mapped_data, message);

    printf("\nData written through memory mapping.\n");


    /* STEP 5: READ THROUGH MEMORY */

    printf("\nMapped File Content:\n");
    printf("%s\n", mapped_data);


    /* STEP 6: SYNCHRONIZE CHANGES */

    if (msync(mapped_data, file_size, MS_SYNC) == -1) {
        perror("msync failed");
    } else {
        printf("\nMemory changes synchronized with file.\n");
    }


    /* STEP 7: UNMAP MEMORY */

    if (munmap(mapped_data, file_size) == -1) {
        perror("munmap failed");
    } else {
        printf("Memory mapping released successfully.\n");
    }


    /* STEP 8: CLOSE FILE */

    close(fd);

    printf("File descriptor closed successfully.\n");


    printf("\nFile created: %s\n", filename);

    printf("\n================================================\n");
    printf("       MEMORY-MAPPED FILE I/O COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   VIRTUAL FILE SYSTEM (VFS) DEMO
   ========================================================= */

void virtual_file_system_demo() {

    FILE *file;
    char line[512];

    struct statvfs fs_info;

    int count = 0;

    printf("\n================================================\n");
    printf("          VIRTUAL FILE SYSTEM DEMO\n");
    printf("================================================\n");


    /* STEP 1: DISPLAY MOUNTED FILE SYSTEMS */

    printf("\n--- MOUNTED FILE SYSTEMS ---\n");

    file = fopen("/proc/mounts", "r");

    if (file == NULL) {

        perror("Unable to open /proc/mounts");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {

        printf("%s", line);

        count++;

        if (count == 10) {
            break;
        }
    }

    fclose(file);


    /* STEP 2: FILE SYSTEM INFORMATION */

    printf("\n--- CURRENT FILE SYSTEM INFORMATION ---\n");

    if (statvfs(".", &fs_info) == -1) {

        perror("statvfs failed");
        return;
    }

    printf("Block Size: %lu bytes\n",
           (unsigned long)fs_info.f_bsize);

    printf("Total Blocks: %lu\n",
           (unsigned long)fs_info.f_blocks);

    printf("Free Blocks: %lu\n",
           (unsigned long)fs_info.f_bfree);

    printf("Available Blocks: %lu\n",
           (unsigned long)fs_info.f_bavail);

    printf("Total File System Size: %lu bytes\n",
           (unsigned long)(fs_info.f_blocks * fs_info.f_frsize));


    /* STEP 3: EXPLAIN VFS */

    printf("\n--- VFS CONCEPT ---\n");

    printf("Linux VFS provides a common interface");
    printf(" for different file systems.\n");

    printf("Applications use common file operations");
    printf(" such as open(), read(), write() and close().\n");

    printf("VFS helps Linux support different file");
    printf(" system implementations.\n");


    printf("\n================================================\n");
    printf("       VIRTUAL FILE SYSTEM DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   EXT4 FILE SYSTEM OVERVIEW DEMO
   ========================================================= */

void ext4_file_system_demo() {

    struct statfs fs;

    const char *path = ".";

    printf("\n================================================\n");
    printf("          EXT4 FILE SYSTEM OVERVIEW\n");
    printf("================================================\n");


    /* STEP 1: GET FILE SYSTEM INFORMATION */

    if (statfs(path, &fs) == -1) {

        perror("statfs failed");
        return;
    }


    /* STEP 2: DISPLAY FILE SYSTEM DETAILS */

    printf("\n--- FILE SYSTEM DETAILS ---\n");

    printf("File System Type ID: 0x%lx\n",
           (unsigned long)fs.f_type);

    printf("Block Size: %ld bytes\n",
           (long)fs.f_bsize);

    printf("Total Blocks: %ld\n",
           (long)fs.f_blocks);

    printf("Free Blocks: %ld\n",
           (long)fs.f_bfree);

    printf("Available Blocks: %ld\n",
           (long)fs.f_bavail);

    printf("Total Inodes: %ld\n",
           (long)fs.f_files);

    printf("Free Inodes: %ld\n",
           (long)fs.f_ffree);


    /* STEP 3: EXPLAIN EXT4 */

    printf("\n--- EXT4 FILE SYSTEM CONCEPT ---\n");

    printf("ext4 is a journaling file system used by Linux.\n");

    printf("It uses inodes to store file metadata.\n");

    printf("It supports large files and file systems.\n");

    printf("Journaling helps maintain file system consistency.\n");

    printf("Extents help manage file data blocks efficiently.\n");


    printf("\n--- IMPORTANT EXT4 FEATURES ---\n");

    printf("1. Journaling\n");
    printf("2. Inode-based metadata\n");
    printf("3. Extents\n");
    printf("4. Delayed allocation\n");
    printf("5. Large file system support\n");


    printf("\n================================================\n");
    printf("       EXT4 FILE SYSTEM DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   THREAD CREATION AND EXECUTION DEMO
   ========================================================= */

void *thread_task(void *arg) {

    char *message = (char *)arg;

    printf("\nThread is executing...\n");

    printf("Thread Message: %s\n", message);

    printf("Thread ID: %lu\n",
           (unsigned long)pthread_self());

    return NULL;
}


/* =========================================================
   THREAD CREATION DEMO
   ========================================================= */

void thread_creation_demo() {

    pthread_t thread;

    int result;

    char message[] = "Priority Scheduling - Thread Execution";

    printf("\n================================================\n");
    printf("          THREAD CREATION DEMO\n");
    printf("================================================\n");


    /* CREATE THREAD */

    printf("\nMain thread: Creating a new thread...\n");

    result = pthread_create(
        &thread,
        NULL,
        thread_task,
        message
    );

    if (result != 0) {

        printf("Thread creation failed: %s\n",
               strerror(result));

        return;
    }

    printf("Thread created successfully!\n");


    /* WAIT FOR THREAD */

    result = pthread_join(thread, NULL);

    if (result != 0) {

        printf("Thread joining failed: %s\n",
               strerror(result));

        return;
    }

    printf("\nMain thread: Child thread completed.\n");

    printf("\n================================================\n");
    printf("       THREAD CREATION DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   MULTIPLE THREAD CREATION DEMO
   ========================================================= */

void *multiple_thread_task(void *arg) {

    int thread_number = *(int *)arg;

    printf("\nThread %d is executing.\n", thread_number);

    printf("Thread %d ID: %lu\n",
           thread_number,
           (unsigned long)pthread_self());

    printf("Thread %d completed its task.\n", thread_number);

    return NULL;
}


void multiple_thread_demo() {

    pthread_t threads[3];

    int thread_numbers[3] = {1, 2, 3};

    int result;

    printf("\n================================================\n");
    printf("          MULTIPLE THREAD DEMO\n");
    printf("================================================\n");


    /* CREATE THREE THREADS */

    for (int i = 0; i < 3; i++) {

        result = pthread_create(
            &threads[i],
            NULL,
            multiple_thread_task,
            &thread_numbers[i]
        );

        if (result != 0) {

            printf("Thread %d creation failed: %s\n",
                   i + 1, strerror(result));

            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            return;
        }

        printf("Thread %d created successfully.\n", i + 1);
    }


    /* WAIT FOR ALL THREADS */

    for (int i = 0; i < 3; i++) {

        pthread_join(threads[i], NULL);
    }


    printf("\nAll three threads completed successfully.\n");

    printf("\n================================================\n");
    printf("       MULTIPLE THREAD DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   RACE CONDITION DEMONSTRATION
   ========================================================= */

int shared_counter = 0;


void *race_condition_task(void *arg) {

    for (int i = 0; i < 1000000; i++) {

        shared_counter++;
    }

    return NULL;
}


void race_condition_demo() {

    pthread_t t1, t2;

    shared_counter = 0;

    printf("\n================================================\n");
    printf("          RACE CONDITION DEMO\n");
    printf("================================================\n");

    printf("\nInitial Counter Value: %d\n", shared_counter);


    /* CREATE TWO THREADS */

    pthread_create(&t1, NULL, race_condition_task, NULL);

    pthread_create(&t2, NULL, race_condition_task, NULL);


    /* WAIT FOR BOTH THREADS */

    pthread_join(t1, NULL);

    pthread_join(t2, NULL);


    printf("\nExpected Counter Value: 2000000\n");

    printf("Actual Counter Value: %d\n", shared_counter);


    printf("\nBoth threads accessed the same shared variable.\n");

    printf("Without synchronization, concurrent updates can");
    printf(" cause a race condition.\n");


    printf("\n================================================\n");
    printf("       RACE CONDITION DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   MUTEX LOCK DEMONSTRATION
   ========================================================= */

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t resource_semaphore;

int mutex_counter = 0;


void *mutex_thread_task(void *arg) {

    for (int i = 0; i < 1000000; i++) {

        /* LOCK THE SHARED RESOURCE */

        pthread_mutex_lock(&counter_mutex);

        mutex_counter++;

        /* UNLOCK THE SHARED RESOURCE */

        pthread_mutex_unlock(&counter_mutex);
    }

    return NULL;
}


void mutex_lock_demo() {

    pthread_t t1, t2;

    mutex_counter = 0;

    printf("\n================================================\n");
    printf("          MUTEX LOCK DEMO\n");
    printf("================================================\n");

    printf("\nInitial Counter Value: %d\n", mutex_counter);


    /* CREATE TWO THREADS */

    pthread_create(&t1, NULL, mutex_thread_task, NULL);

    pthread_create(&t2, NULL, mutex_thread_task, NULL);


    /* WAIT FOR BOTH THREADS */

    pthread_join(t1, NULL);

    pthread_join(t2, NULL);


    printf("\nExpected Counter Value: 2000000\n");

    printf("Actual Counter Value: %d\n", mutex_counter);


    printf("\nMutex lock protects the shared counter.\n");

    printf("Only one thread can access the critical");
    printf(" section at a time.\n");


    /* DESTROY MUTEX */

    


    printf("\n================================================\n");
    printf("          MUTEX LOCK DEMO COMPLETE\n");
    printf("================================================\n");
}


/* =========================================================
   SEMAPHORE THREAD FUNCTION
   ========================================================= */

void *semaphore_thread_task(void *arg) {

    int thread_id = *(int *)arg;

    printf("\nThread %d is waiting for a resource...\n", thread_id);

    /* WAIT FOR SEMAPHORE PERMISSION */

    if (sem_wait(&resource_semaphore) == -1) {
        perror("sem_wait");
        return NULL;
    }

    printf("Thread %d acquired the resource.\n", thread_id);

    /* SIMULATE RESOURCE USAGE */

    sleep(2);

    printf("Thread %d is releasing the resource.\n", thread_id);

    /* RELEASE THE RESOURCE */

    if (sem_post(&resource_semaphore) == -1) {
        perror("sem_post");
    }

    return NULL;
}


/* =========================================================
   SEMAPHORE DEMONSTRATION
   ========================================================= */

void semaphore_demo() {

    pthread_t threads[4];
    int thread_ids[4];

    printf("\n================================================\n");
    printf("          SEMAPHORE DEMO\n");
    printf("================================================\n");

    /* INITIALIZE SEMAPHORE WITH 2 AVAILABLE RESOURCES */

    if (sem_init(&resource_semaphore, 0, 2) == -1) {
        perror("sem_init");
        return;
    }

    printf("\nSemaphore initialized with 2 resources.\n");
    printf("Creating 4 threads...\n");

    /* CREATE FOUR THREADS */

    for (int i = 0; i < 4; i++) {

        thread_ids[i] = i + 1;

        if (pthread_create(&threads[i], NULL,
                           semaphore_thread_task,
                           &thread_ids[i]) != 0) {

            perror("pthread_create");

            /* Wait for threads already created */
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            sem_destroy(&resource_semaphore);
            return;
        }
    }

    /* WAIT FOR ALL THREADS */

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    /* DESTROY SEMAPHORE */

    sem_destroy(&resource_semaphore);

    printf("\nAll four threads completed successfully.\n");
    printf("Maximum simultaneous resource access: 2\n");

    printf("\n================================================\n");
    printf("          SEMAPHORE DEMO COMPLETE\n");
    printf("================================================\n");
}

// Critical Section Demonstration

int critical_section_counter = 0;
pthread_mutex_t critical_section_mutex = PTHREAD_MUTEX_INITIALIZER;

void *critical_section_task(void *arg) {
    int thread_id = *(int *)arg;

    printf("\nThread %d entering critical section...\n", thread_id);

    pthread_mutex_lock(&critical_section_mutex);

    critical_section_counter++;
    printf("Thread %d is executing inside critical section.\n", thread_id);

    pthread_mutex_unlock(&critical_section_mutex);

    printf("Thread %d leaving critical section.\n", thread_id);

    return NULL;
}

void critical_section_demo() {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};

    critical_section_counter = 0;

    printf("\n========== CRITICAL SECTION DEMO ==========\n");

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, critical_section_task, &thread_ids[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinal Critical Section Counter: %d\n", critical_section_counter);

    printf("\n========== CRITICAL SECTION DEMO COMPLETE ==========\n");
}




/* =========================================================
   MAIN MENU
   ========================================================= */

   

int main() {

    Process processes[MAX_PROCESSES];

    int n = 0;
    int choice;

    printf("\n================================================\n");
    printf("          PRIORITY SCHEDULING SYSTEM\n");
    printf("================================================\n");

    printf("\nPriority Rule:\n");
    printf("Smaller priority number = Higher priority\n");

    do {

        printf("\n\n---------------- MAIN MENU ----------------\n");

        printf("1. Enter Process Details\n");
        printf("2. Display Process Details\n");
        printf("3. Non-Preemptive Priority Scheduling\n");
        printf("4. Preemptive Priority Scheduling\n");
        printf("5. Process Management Demo\n");
        printf("6. Anonymous Pipe Demo\n");
        printf("7. Named FIFO Demo\n");
        printf("8. Signal Demo\n");
        printf("9. Shared Memory Demo\n");
        printf("10. Memory Management Demo\n");
printf("11. Copy-on-Write Demo\n");
printf("12. Virtual Memory Demo\n");
printf("13. Page Fault & Demand Paging Demo\n");
printf("14. Memory Errors Demo\n");
printf("15. Memory Debugging & Analysis Demo\n");
printf("16. File System & File I/O Demo\n");
printf("17. File Descriptor & Open File Demo\n");
printf("18. Buffered vs Unbuffered I/O Demo\n");
printf("19. Memory-Mapped File I/O Demo\n");
printf("20. Virtual File System (VFS) Demo\n");
printf("21. ext4 File System Overview Demo\n");
printf("22. Thread Creation Demo\n");
printf("23. Multiple Thread Creation Demo\n");
printf("24. Race Condition Demo\n");
printf("25. Mutex Lock Demo\n");
printf("26. Semaphore Demo\n");
printf("24. Race Condition Demo\n");
printf("25. Mutex Lock Demo\n");
printf("26. Semaphore Demo\n");
printf("27. Critical Section Demo\n");
printf("28. Exit\n");

        printf("--------------------------------------------\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1:

                input_processes(
                    processes,
                    &n
                );

                break;


            case 2:

                display_input(
                    processes,
                    n
                );

                break;


            case 3:

                non_preemptive_priority(
                    processes,
                    n
                );

                break;


            case 4:

                preemptive_priority(
                    processes,
                    n
                );

                break;


        case 5:

            process_management_demo();

            break;


        case 6:
            anonymous_pipe_demo();
            break;

        case 7:
            named_fifo_demo();
            break;

        case 8:
            signal_demo();
            break;

        case 9:
        shared_memory_demo();
        break;  

case 10:
    memory_management_demo();
    break;

case 11:
    copy_on_write_demo();
    break;

case 12:
    virtual_memory_demo();
    break;

case 13:
    page_fault_demo();
    break;

case 14:
    memory_errors_demo();
    break;

case 15:
    memory_debugging_demo();
    break;

case 16:
    file_system_demo();
    break;

case 17:
    file_descriptor_demo();
    break;

case 18:
    buffered_unbuffered_demo();
    break;

case 19:
    memory_mapped_file_demo();
    break;

case 20:
    virtual_file_system_demo();
    break;

case 21:
    ext4_file_system_demo();
    break;

case 22:
    thread_creation_demo();
    break;

case 23:
    multiple_thread_demo();
    break;

case 24:
    race_condition_demo();
    break;

case 25:
    mutex_lock_demo();
    break;

case 26:
    semaphore_demo();
    break;

case 27:
    critical_section_demo();
    break;

case 28:
    printf("\nExiting Priority Scheduling System...\n");
    printf("Thank you!\n");
    break;
            default:

                printf("\nInvalid choice.\n");
                printf("Please select a valid option.\n");
        }

    } while (choice != 28);

    return 0;
}