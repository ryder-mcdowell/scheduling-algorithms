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
  int previously_scheduled_time;
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

  //initialize previous process
  Process previous;
  previous.burst_time = 0;

  //iterate over multimap
  multimap <int, Process> :: iterator itr;
  fprintf(stderr, "======================================\n");
  for (itr = processMultimap.begin(); itr != processMultimap.end(); itr++) {
    //get next process "in queue"
    Process p = itr->second;

    //if arrived after previous was done executing
    if (time_passed <= p.arrival_time){
      time_passed = p.arrival_time;
      total_wait_time -= previous.burst_time;
    }
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //break if sim_time is up
    if (time_passed + p.burst_time > sim_time) {
      time_passed = sim_time;
      if (remaining_tasks > 1) {
        total_wait_time += (time_passed - p.arrival_time);
      }
      fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
      break;
    }

    //add to running totals
    time_passed += p.burst_time;
    throughput += 1;
    remaining_tasks -=1;
    if (remaining_tasks != 0) {
      total_wait_time += (time_passed - p.arrival_time);
    }

    fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
    previous = p;
  }
  fprintf(stderr, "======================================\n");

  outputStats(throughput, total_wait_time, time_passed, remaining_tasks);
}

//shortest-job first algorithm
void shortestJobFirst(int sim_time) {
  multimap <int, Process> processMultimap = mapProcessInput();

  int time_passed = 0;
  int throughput = 0;
  int total_wait_time = 0;
  int remaining_tasks = processMultimap.size();

  //initialize previous process
  Process previous;
  previous.burst_time = 0;

  //go until no more jobs left
  multimap <int, Process> :: iterator itr;
  fprintf(stderr, "======================================\n");
  while (processMultimap.size() > 0) {
    Process p = processMultimap.begin()->second;
    multimap <int, Process> :: iterator itemToDelete = processMultimap.begin();

    //find next shortest job
    Process tmp;
    for (itr = processMultimap.begin(); itr != processMultimap.end(); itr++) {
      tmp = itr->second;
      if (tmp.burst_time < p.burst_time && tmp.arrival_time < time_passed) {
        p = tmp;
        itemToDelete = itr;
      }
    }

    //delete element being scheduled
    processMultimap.erase(itemToDelete);

    //if arrived after previous was done executing
    if (time_passed <= p.arrival_time){
      time_passed = p.arrival_time;
      total_wait_time -= previous.burst_time;
    }
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //break if sim_time is up
    if (time_passed + p.burst_time > sim_time) {
      time_passed = sim_time;
      if (remaining_tasks > 1) {
        total_wait_time += (time_passed - p.arrival_time);
      }
      fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
      break;
    }

    //add to running totals
    time_passed += p.burst_time;
    throughput += 1;
    remaining_tasks -=1;
    if (remaining_tasks != 0) {
      total_wait_time += (time_passed - p.arrival_time);
    }
    fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
    previous = p;
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
  int initial_size = processMultimap.size();
  int counter = 0;

  //initialize previous process
  Process previous;
  previous.burst_time = 0;

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

    //if arrived after previous was done executing
    if (time_passed <= p.arrival_time && time_passed != 0 && counter < initial_size){
      //put on back of queue
      counter += 1;
      processQueue.pop();
      processQueue.push(p);
      continue;
    }
    counter = 0;
    if (time_passed <= p.arrival_time){
      time_passed = p.arrival_time;
    }
    fprintf(stderr, "%7d: Scheduling PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //break if sim_time is up
    if (time_passed + time_slice > sim_time || (time_passed + p.burst_time > sim_time && processQueue.size() == 1)) {
      time_passed = sim_time;
      if (remaining_tasks > 1) {
        total_wait_time += (time_passed - p.previously_scheduled_time - p.arrival_time);
      }
      fprintf(stderr, "%7d:            SIMULATION   terminated\n", time_passed);
      break;
    }

    //if process won't finish before its time is up and isn't the last process
    if (p.burst_time > time_slice && processQueue.size() > 1) {
      //decrease remaining cpu cycles needed for completion
      p.burst_time -= time_slice;

      //add to running totals
      time_passed += time_slice;
      total_wait_time += (time_passed - p.previously_scheduled_time);

      //set previously scheduled time
      p.previously_scheduled_time = time_passed;

      //put on back of queue
      processQueue.pop();
      processQueue.push(p);

      fprintf(stderr, "%7d: Suspending PID %7d, CPU = %7d\n", time_passed, p.process_id, p.burst_time);

    //finishing before time slice is up or is last process, don't suspend
    } else {
      //remove from queue
      processQueue.pop();

      //add to running totals
      time_passed += p.burst_time;
      throughput += 1;
      remaining_tasks -=1;
      if (remaining_tasks != 0) {
        total_wait_time += (time_passed - processQueue.front().previously_scheduled_time - p.arrival_time);
      }

      fprintf(stderr, "%7d:            PID %7d  terminated\n", time_passed, p.process_id);
    }
    previous = p;
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
      p.previously_scheduled_time = 0;
      processes.insert(pair <int, Process> (p.arrival_time, p));
    }
  }
  return processes;
}

void outputStats(int throughput, int total_wait_time, int time_passed, int remaining_tasks) {
  float average_wait_time = 0.0;
  float average_turnaround_time = 0.0;

  fprintf(stderr, "Time passed       =          %7d\n", time_passed);
  fprintf(stderr, "Total Wait Time   =          %7d\n", total_wait_time);
  fprintf(stderr, "Total Turnaround  =          %7d\n\n", time_passed + total_wait_time);

  if (throughput != 0) {
    if (remaining_tasks == 0) {
      //normal case
      average_wait_time = (float)total_wait_time / (float)throughput;
      average_turnaround_time = ((float)time_passed + (float)total_wait_time) / (float)throughput;
    } else {
      //case where simulation was terminated early
      average_wait_time = (float)total_wait_time / ((float)throughput + 1);
      average_turnaround_time = ((float)time_passed + (float)total_wait_time) / ((float)throughput);
    }
  }

  fprintf(stderr, "Throughput =          %10d\n", throughput);
  fprintf(stderr, "Avg Wait Time =       %10f\n", average_wait_time);
  fprintf(stderr, "Avg Turnaround Time = %10f\n", average_turnaround_time);
  fprintf(stderr, "Remaining Tasks =     %10d\n", remaining_tasks);
}
