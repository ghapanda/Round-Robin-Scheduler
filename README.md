# Round-Robin-Scheduler
# You Spin Me Round Robin

Stimulation for round robin scheduling for a given workload and quantum
length. The processes including their arrival time and burst time are read from a file (first argument) and the quantum length is the second argument. Each clock tick is one iteration of the while loop. At the beginning of each clock tick, arriving processes are collected and if they burst time>0, they are added to scheduler queue. Then we let the current running process (if any) runs for that tick, then we decrement the quatum length and time left to complete for that process. If one process is completed or the quantum is over, we increment the number of completed processes, and pop another process of the front of the queue. If there is no running process during a particular clock tick, we do the same. Eventually, we exit the while loop when number of completed process is equal to the number of processes that we had to schedule in the first place.

## Building

```shell
make
```

## Running

cmd for running TODO
```shell
./rr <filename> <quantum length>
./rr processes.txt 3
```

results TODO
```shell
Average waiting time: 7.00
Average response time: 2.75

```

## Cleaning up

```shell
make clean
```
