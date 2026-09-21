//Step 6: Preemptive Priority Scheduling

#include <stdio.h>
#include <stdlib.h>

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


/*
 * Display input process information
 */
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
 * Select the highest-priority available process.
 *
 * Smaller priority number = higher priority.
 */
int select_process(Process p[], int n, int current_time) {

    int selected = -1;

    for (int i = 0; i < n; i++) {

        if (p[i].arrival_time <= current_time &&
            !p[i].completed &&
            p[i].remaining_time > 0) {

            if (selected == -1) {

                selected = i;

            } else if (p[i].priority <
                       p[selected].priority) {

                selected = i;

            } else if (p[i].priority ==
                       p[selected].priority) {

                /*
                 * Tie-breaker:
                 * Earlier arrival time first.
                 */
                if (p[i].arrival_time <
                    p[selected].arrival_time) {

                    selected = i;
                }
            }
        }
    }

    return selected;
}


/*
 * Add a segment to the Gantt chart.
 *
 * Consecutive execution of the same process
 * is merged into one segment.
 */
void add_gantt_entry(
    GanttEntry gantt[],
    int *count,
    int pid,
    int start,
    int end
) {

    if (start == end)
        return;

    if (*count > 0 &&
        gantt[*count - 1].pid == pid &&
        gantt[*count - 1].end_time == start) {

        gantt[*count - 1].end_time = end;

    } else {

        if (*count < MAX_GANTT) {

            gantt[*count].pid = pid;
            gantt[*count].start_time = start;
            gantt[*count].end_time = end;

            (*count)++;
        }
    }
}


/*
 * Display Gantt Chart
 */
void display_gantt(GanttEntry gantt[], int count) {

    printf("\n================================================\n");
    printf("              GANTT CHART\n");
    printf("================================================\n\n");

    printf("|");

    for (int i = 0; i < count; i++) {

        if (gantt[i].pid == -1)
            printf(" IDLE |");
        else
            printf(" P%d |", gantt[i].pid);
    }

    printf("\n");

    if (count > 0) {

        printf("%d", gantt[0].start_time);

        for (int i = 0; i < count; i++) {

            printf("     %d",
                   gantt[i].end_time);
        }

        printf("\n");
    }
}


/*
 * Display final scheduling results
 */
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
    printf("         PREEMPTIVE PRIORITY SCHEDULING\n");
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


    /*
     * Read process information
     */
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

        processes[i].remaining_time =
            processes[i].burst_time;

        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = -1;

        processes[i].started = 0;
        processes[i].completed = 0;
    }


    display_input(processes, n);


    /*
     * Preemptive Priority Scheduling
     *
     * The CPU checks the highest-priority
     * available process at every time unit.
     */
    while (completed < n) {

        int selected =
            select_process(
                processes,
                n,
                current_time
            );


        /*
         * CPU IDLE
         */
        if (selected == -1) {

            int next_arrival = -1;

            for (int i = 0; i < n; i++) {

                if (!processes[i].completed) {

                    if (next_arrival == -1 ||
                        processes[i].arrival_time <
                        next_arrival) {

                        next_arrival =
                            processes[i].arrival_time;
                    }
                }
            }


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
         * First time this process gets CPU
         */
        if (!processes[selected].started) {

            processes[selected].started = 1;

            processes[selected].response_time =
                current_time -
                processes[selected].arrival_time;
        }


        /*
         * Execute the selected process
         * for ONE time unit.
         *
         * This is what allows preemption.
         */
        int start_time = current_time;

        current_time++;

        processes[selected].remaining_time--;


        /*
         * Record this execution in Gantt chart.
         */
        add_gantt_entry(
            gantt,
            &gantt_count,
            processes[selected].pid,
            start_time,
            current_time
        );


        /*
         * Process completed
         */
        if (processes[selected].remaining_time == 0) {

            processes[selected].completed = 1;

            completed++;

            processes[selected].completion_time =
                current_time;

            /*
             * TAT = CT - AT
             */
            processes[selected].turnaround_time =
                processes[selected].completion_time -
                processes[selected].arrival_time;

            /*
             * WT = TAT - BT
             */
            processes[selected].waiting_time =
                processes[selected].turnaround_time -
                processes[selected].burst_time;
        }
    }


    display_results(processes, n);

    display_gantt(gantt, gantt_count);


    printf("\n================================================\n");
    printf("     PREEMPTIVE SCHEDULING COMPLETE\n");
    printf("================================================\n");

    return 0;
}