/*
    Written by: Keith Downie
    uNID: u0494042
    CS 5460 - Operating Systems
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Struct for pthread identification
typedef struct _thread_data_t {
  int tid;
} thread_data_t;

// Struct for the lock
typedef struct {
    volatile int lock;
} mutex;

// mfence function
void mfence (void) {
  asm volatile ("mfence" : : : "memory");
}

/*
 * atomic_xadd
 *
 * equivalent to atomic execution of this code:
 *
 * return (*ptr)++;
 * 
 */
static inline int atomic_xadd (volatile int *ptr)
{
  register int val __asm__("eax") = 1;
  asm volatile ("lock xaddl %0,%1"
  : "+r" (val)
  : "m" (*ptr)
  : "memory"
  );  
  return val;
}

// Global variables to coordinate the spin_lock algorithm
mutex *turn, *number;
volatile int count[99];
volatile int in_cs = 0;
int threads;
double runtime;

/*
    spin_lock - Function to gain control of the lock
    spin_lock_t *s - A pointer to the shared lock object
*/
void spin_lock(mutex *t, mutex *n) {
    int num = atomic_xadd(&n->lock);
    while (num != t->lock) {}
}

/*
    spin_unlock - Function to release control of the lock
    spin_lock_t *s - A pointer to the shared lock object
*/
void spin_unlock(mutex *t) {
    mfence();
    (t->lock)++;
}

/* 
    runThread - Function each threads run, contains the critical section
*/
void *runThread(void *arg) {
    // Get the thread's data struct
    thread_data_t *data = (thread_data_t *)arg;

    // Vars for keeping track of how long to run
    time_t start, end;
    double elapsed;
    start = time(NULL);

    // Loop for the given duration
    while (elapsed < runtime) {
	// Grab the lock
	spin_lock(turn, number);

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
	spin_unlock(turn);

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

    turn = (mutex *) malloc(sizeof(mutex));
    turn->lock = 0;

    number = (mutex *) malloc(sizeof(mutex));
    number->lock = 0;

    // Initialize global count array and create threads
    for (i = 0; i < threads; i++) {
	count[i] = 0;

	// Make a thread and run the spin lock
	thr_data[i].tid = i;
	if ((rc = pthread_create(&thr[i], NULL, runThread, &thr_data[i]))) {
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
    free(turn);
    free(number);

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