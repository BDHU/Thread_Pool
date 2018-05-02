// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include "shared-test.c"

int mutex_flag;

struct work_load {
  pthread_t tid;
  int* start;
  int size;
  int lstart;
};

void* worker_func(void* arg);

int main(int argc, char** argv) {
  int o;
  int workers = 0;
  int test_size = 1000;
  // worker can be predefined or set to default
  struct option opts[3] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((o = getopt_long_only(argc, argv, "w:", opts, NULL)) != -1 ) {
    switch (o) {
      case 0:
      	break;
      case 'w':
        workers = atoi(optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", o);
    }
  }

  // worker version
  workers = workers == 0 ? get_nprocs() : workers;
  printf("Running with %d threads \n", workers);  
  
  struct work_load work[workers];

  int results[test_size];
  for (int i=0; i<test_size; i++) {
    results[i] = i;
  }
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  int work_assigned = 0;
  int average_load = lnum / workers;
  for (int i=0; i<workers; i++) {
    work[i].start = results + work_assigned;
    work[i].size = average_load;
    work[i].lstart = work_assigned;
    if (i == workers - 1) {
      work[i].size = test_size - work_assigned;
    } 
    work_assigned += average_load;

    if (pthread_create(&work[i].tid, NULL, worker_func, work+i) != 0) {
      printf("failed to create thread %d \n", i);
    }
  }

  for (int i=0; i<workers; i++) {
    if (pthread_join(work[i].tid, NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
  }

  // for (int i=0; i<test_size; i++) {
  //   printf("%d ", results[i]);
  // }
  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

// ============== experiment ==================

void* worker_func(void* arg) {
  struct work_load* wl = (struct work_load*)arg;
  struct out o[wl->size];
  for (int i=0; i<wl->size; i++) {
    o[i].dir = "pwoutput";
    o[i].arg = wl->lstart + i;
    
    short_task(wl->start + i);
    long_task(o+i);
  }
  return NULL;
} 