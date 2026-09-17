# Priority Scheduling

## Operating Systems and Systems Programming Project

A CPU Scheduling Simulator implementing **Preemptive and Non-Preemptive Priority Scheduling** in a Linux environment.

---

## 📌 Project Information

| Detail | Information |
|---|---|
| Course | Operating Systems and Systems Programming |
| Course Code | 25CS2104E |
| Academic Year | 2026–27 |
| Term | Term-I |
| Section | Section-6 |
| Team No. | 20 |
| Project Title | Priority Scheduling |
| Faculty | K. Anusha |

---

## 📖 Abstract

Priority Scheduling is a CPU scheduling algorithm used by Operating Systems to allocate the CPU to processes based on their priority levels.

This project focuses on implementing both **Preemptive and Non-Preemptive Priority Scheduling** in a Linux environment using system programming concepts.

The system accepts process details such as:

- Process ID
- Arrival Time
- Burst Time
- Priority

Based on the given process details, the system determines the process execution order and calculates:

- Completion Time
- Turnaround Time
- Waiting Time
- Response Time

The project also displays the process execution order using a **Gantt Chart**.

The proposed solution helps understand how an Operating System manages CPU resources and how priority affects process execution. The expected outcome is a simple and efficient scheduling simulator that demonstrates the practical working and performance of Priority Scheduling.

---

## 🎯 Problem Statement

Processes with different priorities need to be executed in an appropriate order.

Improper scheduling can lead to:

- High waiting time
- High response time
- CPU resource wastage

The project aims to implement **Preemptive and Non-Preemptive Priority Scheduling** in a Linux environment.

It demonstrates process management and CPU scheduling concepts used in Operating Systems and Systems Programming.

---

## 🎯 Objectives

1. To implement Preemptive and Non-Preemptive Priority Scheduling in a Linux environment.
2. To understand how an Operating System allocates CPU resources based on process priorities.
3. To calculate and analyze:
   - Waiting Time
   - Turnaround Time
   - Completion Time
   - Response Time
4. To visualize the process execution sequence using a Gantt Chart and compare scheduling performance.

---

## ⚙️ Proposed Methodology

The project will be designed and implemented in a Linux environment using a suitable programming language such as **C**.

The system will:

1. Accept process details from the user.
2. Take the following inputs:
   - Process ID
   - Arrival Time
   - Burst Time
   - Priority
3. Implement both:
   - Preemptive Priority Scheduling
   - Non-Preemptive Priority Scheduling
4. Simulate CPU allocation according to process priority.
5. Generate the process execution sequence.
6. Calculate:
   - Completion Time
   - Turnaround Time
   - Waiting Time
   - Response Time
7. Display the execution sequence using a Gantt Chart for easy analysis.

---

## 🧠 Scheduling Concepts

### Preemptive Priority Scheduling

In Preemptive Priority Scheduling, the CPU can be taken away from the currently running process when a higher-priority process becomes available.

The project will simulate this behavior based on process arrival time and priority.

### Non-Preemptive Priority Scheduling

In Non-Preemptive Priority Scheduling, once a process starts execution, it continues until completion. The next process is selected based on priority after the current process finishes.

---

## 📊 Process Information

The system accepts the following process information:

| Input | Description |
|---|---|
| Process ID | Unique identifier of the process |
| Arrival Time | Time at which the process arrives |
| Burst Time | CPU time required by the process |
| Priority | Priority level assigned to the process |

---

## 📈 Scheduling Metrics

### 1. Completion Time

The time at which a process completes its execution.

```text
Completion Time (CT)
```

### 2. Turnaround Time

The total time taken by a process from arrival until completion.

```text
Turnaround Time = Completion Time - Arrival Time
```

### 3. Waiting Time

The total time a process spends waiting for CPU execution.

```text
Waiting Time = Turnaround Time - Burst Time
```

### 4. Response Time

The time from when a process arrives until it receives CPU execution for the first time.

```text
Response Time = First CPU Start Time - Arrival Time
```

---

## 📉 Gantt Chart

The project displays the process execution order using a Gantt Chart.

A Gantt Chart provides a visual representation of CPU allocation and helps users understand:

- Which process executes first
- When a process starts
- When a process finishes
- The sequence of process execution
- CPU scheduling behavior

Example representation:

```text
|  P1  |  P2  |  P3  |
0      5      8      12
```

The actual execution order and timings depend on the process input and their priorities.

---

## 🖥️ Operating Systems Concepts / Linux APIs Used

| OS Concept / Linux API / System Call | Purpose in the Project |
|---|---|
| CPU Scheduling | Determines the execution order of processes based on priority |
| Priority Scheduling | Implements Preemptive and Non-Preemptive scheduling |
| Process Management | Manages and simulates the execution of multiple processes |
| Context Switching | Represents switching the CPU between processes |
| `fork()` | Creates processes for demonstrating process management |
| `wait()` | Allows the parent process to wait for child process completion |

---

## 🛠️ Tools / Platforms / Software Used

| Software / Platform | Purpose |
|---|---|
| Linux / Ubuntu | Environment for developing and executing the project |
| C / C++ | Programming language for implementation |
| GCC Compiler | Compiles and executes the source code |
| VS Code / Terminal | Writing, editing and running the program |

---

## 🔄 Project Workflow

```text
                    START
                      |
                      v
             Enter Process Details
                      |
                      v
       +-----------------------------+
       | Process ID                  |
       | Arrival Time                |
       | Burst Time                  |
       | Priority                    |
       +-----------------------------+
                      |
                      v
             Select Scheduling Type
                 /           \
                /             \
               v               v
       Preemptive Priority   Non-Preemptive
          Scheduling            Priority
                              Scheduling
                \             /
                 \           /
                  v         v
             CPU Allocation
                      |
                      v
            Determine Execution Order
                      |
                      v
             Calculate Scheduling
                  Metrics
                      |
                      v
                Gantt Chart
                      |
                      v
                     END
```

---

## 💻 Implementation Environment

The project is intended to be developed and executed in a **Linux / Ubuntu environment**.

The implementation can use **C / C++**, with GCC used for compilation and execution.

Development and execution can be performed using **VS Code or the Linux Terminal**.

---

## 🚀 How to Run

After the source code is added to this repository, compile it using GCC.

### For C

```bash
gcc filename.c -o priority_scheduling
```

Run:

```bash
./priority_scheduling
```

### For C++

```bash
g++ filename.cpp -o priority_scheduling
```

Run:

```bash
./priority_scheduling
```

> Replace `filename.c` or `filename.cpp` with the actual source-code filename used in the project.

---

## 🧪 Expected Input

The system is expected to accept process details such as:

```text
Process ID
Arrival Time
Burst Time
Priority
```

Example:

```text
P1    0    5    2
P2    1    3    1
P3    2    4    3
```

The actual input format will depend on the final implementation.

---

## 📋 Expected Output

The system is expected to display:

1. Process details
2. Process execution order
3. Completion Time
4. Turnaround Time
5. Waiting Time
6. Response Time
7. Gantt Chart

Example result format:

```text
Process    CT    TAT    WT    RT
P1         --    --     --    --
P2         --    --     --    --
P3         --    --     --    --
```

Example Gantt Chart:

```text
|  P1  |  P2  |  P3  |
0      --     --     --
```

The actual values depend on the input processes and their priority levels.

---

## 👥 Team Members

| Roll Number | Student Name | Individual Responsibility |
|---|---|---|
| 2520030460 | K. Poojitha Rani | Implement Priority Scheduling algorithm |
| 2520030487 | Somisetty Ketana Sindhu | Implement process management and Linux APIs |
| 2520030615 | Yerrolla Eekshitha | Testing, documentation and presentation |

---

## 👩‍💻 Individual Contributions

### K. Poojitha Rani
- Implement Priority Scheduling algorithm

### Somisetty Ketana Sindhu
- Implement process management
- Work with Linux APIs

### Yerrolla Eekshitha
- Testing
- Documentation
- Presentation

---

## 📁 Suggested Repository Structure

```text
2520030487_615_OS_PROJECT/
│
├── README.md
├── src/
│   └── priority_scheduling.c
│
├── screenshots/
│   └── output.png
│
├── docs/
│   └── project-report.pdf
│
└── presentation/
    └── project-presentation.pdf
```

> Update the structure when the actual source files, screenshots, report, and presentation are added.

---

## 🎓 Expected Outcome

The project is expected to successfully implement **Preemptive and Non-Preemptive Priority Scheduling** in a Linux environment.

It will:

- Accept process details
- Determine execution order based on priority
- Display the process execution sequence through a Gantt Chart
- Calculate Completion Time
- Calculate Turnaround Time
- Calculate Waiting Time
- Calculate Response Time

The project will help users understand and analyze the performance of Priority Scheduling and understand how an Operating System manages CPU resources.

---

## 🔗 GitHub Repository

**Repository:**

https://github.com/ketanasindhusomisetty/2520030487_615_OS_PROJECT

---

## 📚 Academic Purpose

This project is developed as part of the **Operating Systems and Systems Programming (25CS2104E)** course for the academic year **2026–27, Term-I**.

The project demonstrates practical concepts related to:

- CPU Scheduling
- Priority Scheduling
- Process Management
- Context Switching
- Linux System Programming
- `fork()`
- `wait()`

---

## 👩‍🏫 Faculty

**Faculty Name:** K. Anusha

---

## 📜 License

This project is developed for academic purposes as part of the
**Operating Systems and Systems Programming** course.

---

## ⭐ Project Summary

**Priority Scheduling** is a CPU scheduling simulator that demonstrates how process priorities can be used to determine CPU execution order.

The project covers both **Preemptive and Non-Preemptive Priority Scheduling**, calculates important scheduling metrics, demonstrates process management concepts using Linux APIs, and visualizes execution using a Gantt Chart.

**Course:** Operating Systems and Systems Programming  
**Project:** Priority Scheduling  
**Team:** 20  
**Section:** Section-6
