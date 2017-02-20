/*
    Written by: Keith Downie
    uNID: u0494042
    CS 5460 - Operating Systems
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

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
volatile uint64_t in_circle = 0;
volatile uint64_t total = 0;
int threads;
double runtime;

/*
    spin_lock - Function to gain control of the lock
    spin_lock_t *s - A pointer to the shared lock object
*/
void spin_lock(mutex *t, mutex *n) {
    int num = atomic_xadd(&n->lock);
    while (num != t->lock) {
	if (sched_yield() < 0) {
	    fprintf(stderr, "Failed to yield!");
	    exit(EXIT_FAILURE);
	}
    }
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
    withinCircle - Calculate if the point is within a unit circle
    returns 0 or 1
*/
int withinCircle(double x, double y) {
    if (x*x+y*y<=1) {
	return 1;
    }
    return 0;
}

/* 
    runThread - Function each threads run, contains the critical section
*/
void *runThread() {
    // Vars for keeping track of how long to run
    time_t start, end;
    double elapsed;
    start = time(NULL);

    // Loop for the given duration
    while (elapsed < runtime) {
	// Generate two random doubles and map them to [0,1]
	double x = (double)rand() / (double)RAND_MAX;
	double y = (double)rand() / (double)RAND_MAX;

	// See if these points are inside of the unit circle
	int result = withinCircle(x, y);
	
	// Grab the lock before modifying global variables
	spin_lock(turn, number);
	
	// Critical section
	// Used to assert that the lock is working
	uint64_t tmpt = total;
	uint64_t tmpi = in_circle;

	// Modify global variables
	total++;
	in_circle += result;
	assert(total == tmpt+1);
	assert(in_circle == tmpi+result);
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

    turn = (mutex *) malloc(sizeof(mutex));
    turn->lock = 0;

    number = (mutex *) malloc(sizeof(mutex));
    number->lock = 0;

    srand(time(NULL));

    // Initialize bakery arrays and create threads
    for (i = 0; i < threads; i++) {
	// Make a thread and run the spin lock
	if ((rc = pthread_create(&thr[i], NULL, runThread, NULL))) {
	  fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	  exit(EXIT_FAILURE);
	}
    }

    // Wait until all threads are done
    for (i = 0; i < threads; i++) {
	pthread_join(thr[i], NULL);
    }

    // Generate the result from the global variables
    double result = 4.0*((double)in_circle/(double)total);
    // Print the result
    printf("%f\n", result);

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