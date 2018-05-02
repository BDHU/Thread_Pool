// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>

int mutex_flag;
const int prime = 4222234741;
const char* data = "A purely peer-to-peer version of electronic cash would allow online\
payments to be sent directly from one party to another without going through a\
financial institution. Digital signatures provide part of the solution, but the main\
benefits are lost if a trusted third party is still required to prevent double-spending.\
We propose a solution to the double-spending problem using a peer-to-peer network.\
The network timestamps transactions by hashing them into an ongoing chain of\
hash-based proof-of-work, forming a record that cannot be changed without redoing\
the proof-of-work. The longest chain not only serves as proof of the sequence of\
events witnessed, but proof that it came from the largest pool of CPU power. As\
long as a majority of CPU power is controlled by nodes that are not cooperating to\
attack the network, they'll generate the longest chain and outpace attackers. The\
network itself requires minimal structure. Messages are broadcast on a best effort\
basis, and nodes can leave and rejoin the network at will, accepting the longest\
proof-of-work chain as proof of what happened while they were gone. \n";

void short_task(void* arg);
void long_task(void* arg);
void* worker_func(void* arg);

struct work_load {
  pthread_t tid;
  int* start;
  int size;
  int lstart;
};

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
  struct work_load work[test_size];
  // pthread_t tids[test_size];
  int results[test_size];
  for (int i=0; i<test_size; i++) {
    results[i] = i;
  }
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  for (int i=0; i<test_size; i++) {
    work[i].start = results + i;
    work[i].lstart = i;
    if (pthread_create(&work[i].tid, NULL, worker_func, work+i) != 0) {
      printf("failed to create thread %d \n", i);
    }
  }

  for (int i=0; i<test_size; i++) {
    if (pthread_join(work[i].tid, NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
  }

  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

// ============== experiment ==================

void long_task(void* arg) {
  char buf[30];
  snprintf(buf, 30, "poutput/tmp-%d", (int)arg);
  
  FILE *f = fopen(buf, "w+");
  if (!f) {
    printf("Failed to create file \n");
    return;
  }

  size_t size = strlen(data)+1;
  size_t wsize = fwrite(data, 1, size, f);
  if (wsize != size)
    printf("Failed to write %lu bytes, only wrote %lu \n", size, wsize);
} 

// compute sum and then hash it
void short_task(void* arg) {
  int sum = 0;
  int* p = (int*) arg;
  // compute sum 
  for (int x=1; x<*p; x++) 
    sum += x;
  *p = (sum * (sum-2)) % prime;
}

void* worker_func(void* arg) {
  struct work_load* wl = (struct work_load*)arg;
  short_task(wl->start);
  long_task(wl->lstart);
  return NULL;
} 