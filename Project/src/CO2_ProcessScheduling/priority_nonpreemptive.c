//Step5 - Priority Scheduling
//Non-Preemptive Priority Scheduling

/*
Priority 1 → highest
Priority 2 → next
Priority 3 → next
Priority 4 → lower
*/
#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 20
#define MAX_GANTT 100

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

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


/* Display input process information */
void display_input(Process p[], int n) {

    printf("\n================================================\n");
    printf("              PROCESS INFORMATION\n");
    printf("================================================\n");

    printf("%-8s %-8s %-8s %-10s\n",
           "PID", "AT", "BT", "Priority");

    printf("------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("%-8d %-8d %-8d %-10d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].priority);
    }

    printf("================================================\n");
}


/*
 * Select the highest-priority process
 * among the processes that have arrived.
 *
 * Smaller priority number = higher priority.
 */
int select_process(Process p[], int n, int current_time) {

    int selected = -1;

    for (int i = 0; i < n; i++) {

        if (p[i].arrival_time <= current_time &&
            !p[i].completed) {

            if (selected == -1) {

                selected = i;

            } else if (p[i].priority < p[selected].priority) {

                selected = i;

            } else if (p[i].priority == p[selected].priority) {

                /* Earlier arrival time wins the tie */
                if (p[i].arrival_time <
                    p[selected].arrival_time) {

                    selected = i;
                }
            }
        }
    }

    return selected;
}


/* Display the Gantt Chart */
void display_gantt(GanttEntry gantt[], int count) {

    printf("\n================================================\n");
    printf("                 GANTT CHART\n");
    printf("================================================\n\n");

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
        printf("     %d", gantt[i].end_time);
    }

    printf("\n");
}


/* Display final scheduling results */
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


int main() {

    Process processes[MAX_PROCESSES];
    GanttEntry gantt[MAX_GANTT];

    int n;
    int current_time = 0;
    int completed = 0;
    int gantt_count = 0;

    printf("================================================\n");
    printf("       NON-PREEMPTIVE PRIORITY SCHEDULING\n");
    printf("================================================\n");

    printf("\nPriority Rule:\n");
    printf("Smaller priority number = Higher priority\n");

    printf("\nEnter number of processes (1-%d): ",
           MAX_PROCESSES);

    scanf("%d", &n);

    if (n < 1 || n > MAX_PROCESSES) {

        printf("Invalid number of processes.\n");
        return 1;
    }


    /* Read process information */
    for (int i = 0; i < n; i++) {

        printf("\nProcess %d\n", i + 1);

        printf("PID          : ");
        scanf("%d", &processes[i].pid);

        printf("Arrival Time : ");
        scanf("%d", &processes[i].arrival_time);

        printf("Burst Time   : ");
        scanf("%d", &processes[i].burst_time);

        printf("Priority     : ");
        scanf("%d", &processes[i].priority);

        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = -1;

        processes[i].started = 0;
        processes[i].completed = 0;
    }


    display_input(processes, n);


    /*
     * Non-Preemptive Priority Scheduling
     */
    while (completed < n) {

        int selected =
            select_process(processes, n, current_time);


        /*
         * No process has arrived yet.
         * Move CPU time to the next arrival.
         */
        if (selected == -1) {

            int next_arrival = -1;

            for (int i = 0; i < n; i++) {

                if (!processes[i].completed) {

                    if (next_arrival == -1 ||
                        processes[i].arrival_time < next_arrival) {

                        next_arrival =
                            processes[i].arrival_time;
                    }
                }
            }


            /* Record CPU idle time */
            if (next_arrival > current_time) {

                if (gantt_count < MAX_GANTT) {

                    gantt[gantt_count].pid = -1;

                    gantt[gantt_count].start_time =
                        current_time;

                    gantt[gantt_count].end_time =
                        next_arrival;

                    gantt_count++;
                }

                current_time = next_arrival;
            }

            continue;
        }


        /*
         * Selected process starts execution.
         */
        processes[selected].started = 1;


        /*
         * Response Time =
         * First CPU Start Time - Arrival Time
         */
        processes[selected].response_time =
            current_time -
            processes[selected].arrival_time;


        /*
         * Start Gantt entry.
         */
        if (gantt_count < MAX_GANTT) {

            gantt[gantt_count].pid =
                processes[selected].pid;

            gantt[gantt_count].start_time =
                current_time;
        }


        /*
         * NON-PREEMPTIVE:
         *
         * Once the process starts,
         * it runs until completion.
         */
        current_time +=
            processes[selected].burst_time;


        /*
         * Completion Time
         */
        processes[selected].completion_time =
            current_time;


        /*
         * Turnaround Time
         *
         * TAT = CT - AT
         */
        processes[selected].turnaround_time =
            processes[selected].completion_time -
            processes[selected].arrival_time;


        /*
         * Waiting Time
         *
         * WT = TAT - BT
         */
        processes[selected].waiting_time =
            processes[selected].turnaround_time -
            processes[selected].burst_time;


        /* Process has completed */
        processes[selected].completed = 1;

        completed++;


        /*
         * Finish Gantt entry.
         */
        if (gantt_count < MAX_GANTT) {

            gantt[gantt_count].end_time =
                current_time;

            gantt_count++;
        }
    }


    /* Display final results */
    display_results(processes, n);

    /* Display Gantt Chart */
    display_gantt(gantt, gantt_count);


    printf("\n================================================\n");
    printf("   NON-PREEMPTIVE SCHEDULING COMPLETE\n");
    printf("================================================\n");

    return 0;
}