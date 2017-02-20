/*
    Written by: Keith Downie
    uNID: u0494042
    CS 5460 - Operating Systems
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Global variables to coordinate the bakery algorithm
volatile int n[99];
volatile int c[99];
volatile int count[99];
volatile int in_cs = 0;
volatile int place = 0;
int threads;
double runtime;

// Struct for pthread identification
typedef struct _thread_data_t {
  int tid;
} thread_data_t;

/*
    getLock - Function to gain control of the lock
    int t - ID number of the thread
*/ 
void getLock(int t) {
    int j;
    // Signal that a number is being taken
    c[t] = 1;
    // Get a place in line
    n[t] = place++;
    // Signal that a number has been taken
    c[t] = 0;

    // Loop through until it is this thread's turn
    for (j = 0; j < threads; j++) {
	while (c[j]) {}

	while ((n[j] != 0) && (n[j] < n[t] || (n[j] == n[t] && j < t))) {}
    }
}

/*
    releaseLock - Function to release control of the lock
    int t - ID number of the thread
*/ 
void releaseLock(int t) {
    // Set thread's number back to 0
    n[t] = 0;
}

/* 
    bakery - Function each threads run, contains the critical section
*/
void *bakery(void *arg) {
    // Get the thread's data struct
    thread_data_t *data = (thread_data_t *)arg;

    // Vars for keeping track of how long to run
    time_t start, end;
    double elapsed;
    start = time(NULL);

    // Loop for the given duration
    while (elapsed < runtime) {
	// Grab the lock
	getLock(data->tid);

	// Critical section
	count[data->tid]++;
	assert (in_cs==0);
	in_cs++;
	assert (in_cs==1);
	in_cs++;
	assert (in_cs==2);
	in_cs++;
	assert (in_cs==3);
	in_cs=0;
	// End critical section

	// Release the lock
	releaseLock(data->tid);
	// Update the run time
	end = time(NULL);
	elapsed = difftime(end, start);
    }
 
    // Exit the thread
    pthread_exit(NULL);
}

/* 
    run - Setup function for the main process
    Sets up the global variables and spawns the threads
*/
int run() {
    int i, rc;
    pthread_t thr[threads];
    thread_data_t thr_data[threads];

    // Initialize bakery arrays and create threads
    for (i = 0; i < threads; i++) {
	// Initialize global variables
	n[i] = 0;
	c[i] = 0;
	count[i] = 0;

	// Make a thread and run the bakery
	thr_data[i].tid = i;
	if ((rc = pthread_create(&thr[i], NULL, bakery, &thr_data[i]))) {
	  fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	  exit(EXIT_FAILURE);
	}
    }

    // Wait until all threads are done
    for (i = 0; i < threads; i++) {
	pthread_join(thr[i], NULL);
	// Print how many times each thread entered the critical section
	printf("Thread %d entered %d times\n", i, count[i]);
    }

    return 0;
}

/*
    main - Starts the program
*/
int main(int argc, char *argv[]) {
    if (argc) {}
    // Set the global variables
    threads = atoi(argv[1]);
    runtime = atof(argv[2]);
    if (threads > 0 && runtime > 0) {
	run();
    }

    return 0;
}