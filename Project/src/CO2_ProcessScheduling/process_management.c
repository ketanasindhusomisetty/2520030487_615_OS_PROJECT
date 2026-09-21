//Step4 - Process management

#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 10

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


/*
 * Display the process information
 */
void display_processes(Process processes[], int count) {

    printf("\n===============================================================\n");
    printf("                    PROCESS TABLE\n");
    printf("===============================================================\n");

    printf("%-8s %-8s %-8s %-10s %-10s %-10s\n",
           "PID",
           "AT",
           "BT",
           "Priority",
           "Remaining",
           "State");

    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {

        const char *state;

        if (processes[i].completed)
            state = "TERMINATED";
        else if (processes[i].started)
            state = "RUNNING";
        else
            state = "READY";

        printf("%-8d %-8d %-8d %-10d %-10d %-10s\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority,
               processes[i].remaining_time,
               state);
    }

    printf("===============================================================\n");
}


/*
 * Initialize process information
 */
void initialize_process(Process *p,
                         int pid,
                         int arrival_time,
                         int burst_time,
                         int priority) {

    p->pid = pid;
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->priority = priority;

    p->remaining_time = burst_time;

    p->completion_time = 0;
    p->turnaround_time = 0;
    p->waiting_time = 0;
    p->response_time = -1;

    p->started = 0;
    p->completed = 0;
}


int main() {

    Process processes[MAX_PROCESSES];

    int n;

    printf("========================================\n");
    printf("       CO-2: PROCESS MANAGEMENT\n");
    printf("========================================\n\n");

    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    scanf("%d", &n);

    if (n < 1 || n > MAX_PROCESSES) {

        printf("Invalid number of processes.\n");
        return 1;
    }


    /*
     * Read process information
     */
    for (int i = 0; i < n; i++) {

        int pid;
        int arrival_time;
        int burst_time;
        int priority;

        printf("\nProcess %d\n", i + 1);

        printf("PID          : ");
        scanf("%d", &pid);

        printf("Arrival Time : ");
        scanf("%d", &arrival_time);

        printf("Burst Time   : ");
        scanf("%d", &burst_time);

        printf("Priority     : ");
        scanf("%d", &priority);

        initialize_process(
            &processes[i],
            pid,
            arrival_time,
            burst_time,
            priority
        );
    }


    /*
     * Display the managed processes
     */
    display_processes(processes, n);


    printf("\nProcess management structure created successfully.\n");

    printf("\nEach process currently contains:\n");
    printf("  - Process ID\n");
    printf("  - Arrival Time\n");
    printf("  - Burst Time\n");
    printf("  - Priority\n");
    printf("  - Remaining Time\n");
    printf("  - Scheduling metrics\n");
    printf("  - Process state\n");


    printf("\n========================================\n");
    printf("       PROCESS MANAGEMENT COMPLETE\n");
    printf("========================================\n");

    return 0;
}