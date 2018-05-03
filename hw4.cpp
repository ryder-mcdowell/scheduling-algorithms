#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <iterator>
#include <vector>
#include <queue>
#include <cstring>
using namespace std;

class Input {
public:
    int sim_time;
    char *algorithm;
    int time_slice;
};

class Process {
public:
  int process_id;
  int arrival_time;
  int burst_time;
};

void checkInputArgs(int argc, char **argv);
Input storeInputArgs(int argc, char **argv);
void firstComeFirstServe(int sim_time);
void shortestJobFirst(int sim_time);
void roundRobin(int sim_time, int time_slice);
multimap <int, Process> mapProcessInput();
void outputStats(int throughput, int total_wait_time, int time_passed, int remaining_tasks);


int main(int argc, char **argv) {

  checkInputArgs(argc, argv);
  Input input = storeInputArgs(argc, argv);

  if (!strcmp(input.algorithm, "FCFS")) {
    firstComeFirstServe(input.sim_time);
  }
  if (!strcmp(input.algorithm, "SJF")) {
    shortestJobFirst(input.sim_time);
  }
  if (!strcmp(input.algorithm, "RR")) {
    roundRobin(input.sim_time, input.time_slice);
  }
}


//exits if input arguments do not follow usage
void checkInputArgs(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "usage: ./a.out sim_time algorithm [time_slice]\n");
    exit(1);
  }
  if (atoi(argv[1]) < 1) {
    fprintf(stderr, "error: sim_time cannot be less than 1\n");
    exit(1);
  }
  if (strcmp(argv[2], "FCFS") && strcmp(argv[2], "SJF") && strcmp(argv[2], "RR")) {
    fprintf(stderr, "error: unrecognized algorithm type\n");
    exit(1);
  }
  if (!strcmp(argv[2], "RR") && argc == 3) {
    fprintf(stderr, "error: must provide time slice with RR algorithm type\n");
    exit(1);
  }
  if (argc == 4) {
    if (atoi(argv[3]) < 1) {
      fprintf(stderr, "error: time_slice cannot be less than 1\n");
      exit(1);
    }
  }
}

//stores and return input arguments in Input object
Input storeInputArgs(int argc, char **argv) {
  Input input;
  input.sim_time = atoi(argv[1]);
  input.algorithm = argv[2];
  if (argc == 4) {
    input.time_slice = atoi(argv[3]);
  }
  return input;
}

//first-come first-serve algorithm
void firstComeFirstServe(int sim_time) {
  multimap <int, Process> processMultimap = mapProcessInput();

  int time_passed = 0;
  int throughput = 0;
  int total_wait_time = 0;
  int remaining_tasks = processMultimap.size();

  //iterate over multimap
  multimap <int, Process> :: iterator itr;
  fprintf(stderr, "======================================\n");
  for (itr = processMultimap.begin(); itr != processMultimap.end(); itr++) {
    //get next process "in queue"
    Process p = itr->second;
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //break if sim_time is up
    if (time_passed + p.burst_time > sim_time) {
      time_passed = sim_time;
      fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
      break;
    }

    //add to running totals
    time_passed += p.burst_time;
    throughput += 1;
    remaining_tasks -=1;
    if (remaining_tasks != 0) {
      total_wait_time += time_passed;
    }

    fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
  }
  fprintf(stderr, "======================================\n");
  outputStats(throughput, total_wait_time, time_passed, remaining_tasks);
}

//shortest-job first algorithm
void shortestJobFirst(int sim_time) {
  multimap <int, Process> processMultimap = mapProcessInput();
  multimap <int, Process> processMultimap2;

  int time_passed = 0;
  int throughput = 0;
  int total_wait_time = 0;
  int remaining_tasks = processMultimap.size();

  //iterate over multimap
  multimap <int, Process> :: iterator itr;
  multimap <int, Process> :: iterator itr2;
  fprintf(stderr, "======================================\n");
  for (itr = processMultimap.begin(); itr != processMultimap.end(); itr++) {
    Process p = itr->second;
    processMultimap2.insert(pair <int, Process> (p.burst_time, p));

    //add all the processes in the first multimap that have the same arrival_time into a second multimap
    for (int i = 0; i < processMultimap.count(p.arrival_time) - 1; i++) {
      itr++;
      Process p = itr->second;
      processMultimap2.insert(pair <int, Process> (p.burst_time, p));
    }

    //iterate over all elements in second multimap
    for (itr2 = processMultimap2.begin(); itr2 != processMultimap2.end(); itr2++) {
      //get next process "in queue"
      Process p = itr2->second;
      fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

      //break if sim_time is up
      if (time_passed + p.burst_time > sim_time) {
        time_passed = sim_time;
        fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
        break;
      }
      //add to running totals
      time_passed += p.burst_time;
      throughput += 1;
      remaining_tasks -=1;
      if (remaining_tasks != 0) {
        total_wait_time += time_passed;
      }

      fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
    }
    //clear multimap for second set of processes with the same arrival time
    processMultimap2.clear();
  }
  fprintf(stderr, "======================================\n");
  outputStats(throughput, total_wait_time, time_passed, remaining_tasks);
}

//round-robin algorithm
void roundRobin(int sim_time, int time_slice) {
  multimap <int, Process> processMultimap = mapProcessInput();
  queue <Process> processQueue;

  int time_passed = 0;
  int throughput = 0;
  int total_wait_time = 0;
  int remaining_tasks = processMultimap.size();
  int accumulator = 0;

  //pull processes from multimap into queue
  multimap <int, Process> :: iterator itr;
  for (itr = processMultimap.begin(); itr != processMultimap.end(); ++itr) {
    processQueue.push(itr->second);
  }

  //cycle until queue is empty
  fprintf(stderr, "======================================\n");
  while (processQueue.size() > 0) {
    //get next in queue
    Process p = processQueue.front();
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //break if sim_time is up
    if (time_passed + time_slice > sim_time || (time_passed + p.burst_time > sim_time && processQueue.size() == 1)) {
      time_passed = sim_time;
      fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
      break;
    }

    //if process won't finish before its time is up and isn't the last process
    if (p.burst_time > time_slice && processQueue.size() > 1) {
      //decrease remaining cpu cycles needed for completion
      p.burst_time -= time_slice;
      fprintf(stderr, "%7d: Suspending PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

      //put on back of queue
      processQueue.pop();
      processQueue.push(p);

      //add to running totals
      time_passed += time_slice;
      total_wait_time += time_slice;  //(time_slice * accumulator)?
    } else {
      //remove from queue
      processQueue.pop();

      //add to running totals
      time_passed += p.burst_time;
      throughput += 1;
      remaining_tasks -=1;
      total_wait_time += time_slice;  //(time_slice * accumulator)?

      fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
    }
  }
  fprintf(stderr, "======================================\n");
  outputStats(throughput, total_wait_time, time_passed, remaining_tasks);

}

//stores and returns process info from cin into a multimap
multimap <int, Process> mapProcessInput() {
  vector <int> v;
  multimap <int, Process> processes;

  //recieve input from stdin and store in vector
  /////////// while (cin >> x >> y >> z) {}
  int tmp;
  while (cin >> tmp) {
    v.push_back(tmp);
  }

  //every third element of vector, take 3 previous elements, create object, and store in multimap
  for (int i = 1; i <= v.size(); i++) {
    if (i % 3 == 0) {
      Process p;
      p.process_id = v[i - 3];
      p.arrival_time = v[i - 2];
      p.burst_time = v[i - 1];
      processes.insert(pair <int, Process> (p.arrival_time, p));
    }
  }
  return processes;
}

void outputStats(int throughput, int total_wait_time, int time_passed, int remaining_tasks) {
  int average_wait_time;
  int average_turnaround_time;

  fprintf(stderr, "Time passed       =          %7d\n", time_passed);
  fprintf(stderr, "Total Weight Time =          %7d\n", total_wait_time);
  fprintf(stderr, "Total Turnaround  =          %7d\n\n", time_passed + total_wait_time);

  if (throughput != 0) {
    if (remaining_tasks == 0) {
      //normal case
      average_wait_time = total_wait_time / throughput;
      average_turnaround_time = (time_passed + total_wait_time) / throughput;
    } else {
      //case where simulation was terminated early
      average_wait_time = total_wait_time / (throughput + 1);
      average_turnaround_time = (time_passed + total_wait_time) / (throughput + 1);
    }
  }

  fprintf(stderr, "Throughput =          %7d\n", throughput);
  fprintf(stderr, "Avg Wait Time =       %7d\n", average_wait_time);
  fprintf(stderr, "Avg Turnaround Time = %7d\n", average_turnaround_time);
  fprintf(stderr, "Remaining Tasks =     %7d\n", remaining_tasks);
}
