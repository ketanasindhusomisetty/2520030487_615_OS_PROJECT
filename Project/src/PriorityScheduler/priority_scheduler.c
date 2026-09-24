#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_PROCESSES 20
#define MAX_GANTT 200

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
        printf("8. Exit\n");

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
            printf("\nExiting Priority Scheduling System...\n");
            printf("Thank you!\n");
            break;

            default:

                printf("\nInvalid choice.\n");
                printf("Please select a valid option.\n");
        }

    } while (choice != 10);

    return 0;
}