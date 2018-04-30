#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <iterator>
#include <vector>
#include <queue>
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
void firstComeFirstServe();
multimap <int, Process> mapProcessesInputBy(const char *key);
void outputStats(int throughput, int total_wait_time, int time_passed, int remaining_tasks);


int main(int argc, char **argv) {

  checkInputArgs(argc, argv);
  Input input = storeInputArgs(argc, argv);

  if (!strcmp(input.algorithm, "FCFS")) {
    firstComeFirstServe();
  }
  if (!strcmp(input.algorithm, "SJF")) {

  }
  if (!strcmp(input.algorithm, "RR")) {

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
void firstComeFirstServe() {
  multimap <int, Process> processMultimap = mapProcessesInputBy("arrival_time");
  queue <Process> processQueue;

  //pull processes from multimap into queue
  multimap <int, Process> :: iterator itr;
  for (itr = processMultimap.begin(); itr != processMultimap.end(); ++itr) {
    cout << itr->second.process_id << " " << itr->first << itr->second.arrival_time << " " << itr->second.burst_time << '\n';
    processQueue.push(itr->second);
  }

  int time_passed = 0;
  int throughput = 0;
  int total_wait_time = 0;
  //int total_turnaround_time = 0;
  int remaining_tasks = processQueue.size();

  fprintf(stderr, "======================================\n");
  while (processQueue.size() > 0) {
    //get first in queue
    Process p = processQueue.front();
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //do "work"
    sleep(p.burst_time * 0.1);     //!!!!

    //remove element and add to running totals;
    processQueue.pop();
    time_passed += p.burst_time;
    throughput += 1;
    remaining_tasks -=1;
    if (processQueue.size() >= 1) {
      total_wait_time += time_passed;
    }

    fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
  }
  fprintf(stderr, "======================================\n");

  outputStats(throughput, total_wait_time, time_passed, remaining_tasks);
}

//stores and returns process info from cin into a multimap
multimap <int, Process> mapProcessesInputBy(const char *key) {
  vector <int> v;
  multimap <int, Process> processes;

  //recieve input from stdin and store in vector
  int tmp;
  while (cin >> tmp) {
    v.push_back(tmp);
  }

  //every third element of vector, take 3 previous elements, create object, and store in multimap
  for (int i = 1; i <= v.size(); i++) {
    if (i % 3 == 0) {
      Process p;
      fprintf(stderr, "%d %d %d\n", v[i - 3], v[i - 2], v[i - 1]);
      p.process_id = v[i - 3];
      p.arrival_time = v[i - 2];
      p.burst_time = v[i - 1];
      //map based on key provided by function parameter (for different sorts based on algorithm)
      if (!strcmp(key, "arrival_time")) {
        processes.insert(pair <int, Process> (p.arrival_time, p));
      } else if (!strcmp(key, "burst_time")) {
        processes.insert(pair <int, Process> (p.burst_time, p));
      }
    }
  }

  return processes;
}

void outputStats(int throughput, int total_wait_time, int time_passed, int remaining_tasks) {
  int average_wait_time;
  int average_turnaround_time;

  fprintf(stderr, "Total Weight Time =          %7d\n", total_wait_time);
  fprintf(stderr, "Total Turnaround  =          %7d\n", time_passed);

  average_wait_time = total_wait_time / throughput;
  average_turnaround_time = time_passed / throughput;

  fprintf(stderr, "Throughput =          %7d\n", throughput);
  fprintf(stderr, "Avg Wait Time =       %7d\n", average_wait_time);
  fprintf(stderr, "Avg Turnaround Time = %7d\n", average_turnaround_time);
  fprintf(stderr, "Remaining Tasks =     %7d\n", remaining_tasks);
}
