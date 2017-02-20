/*
	Assignment 4 - problem_1.c
	Created by Keith Downie
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

// Global variables for thread use
volatile int n_cats, n_dogs, n_birds;
volatile int cats, dogs, birds;
volatile int p_cats, p_dogs, p_birds;
volatile int quit;
pthread_mutex_t lock;
pthread_cond_t cvC;
pthread_cond_t cvDB;

// Struct that gets assigned to each thread
typedef struct _thread_data_t {
	int animalType;
} thread_data_t;

/* 
	play

	Function required by the assignment to ensure
	nothing is violated.
*/
void play(void) {
  for (int i=0; i<10; i++) {
    assert(cats >= 0 && cats <= n_cats);
    assert(dogs >= 0 && dogs <= n_dogs);
    assert(birds >= 0 && birds <= n_birds);
    assert(cats == 0 || dogs == 0);
    assert(cats == 0 || birds == 0);
   }
}

/*
	cat_exit

	Function that is called when a cat has finished playing
*/
void cat_exit(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Decrement the amount of cats playing
	cats--;
	// Send the condition for Birds and Dogs to wake
	// Birds and Dogs will then check if there are cats playing
	pthread_cond_signal(&cvDB);
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void cat_enter(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Check condition.
	// Ensure there are no dogs or birds playing.
	// Also check if the monitor has instructed the thread to quit.
	while (birds > 0 || dogs > 0 || quit == 1) {	
		// If the monitor has set this, exit the thread.
		if (quit == 1) {
			// Signal everyone to wake so they can quit.
			pthread_cond_signal(&cvC);
			pthread_cond_signal(&cvDB);

			// Release the lock
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
		}
		// Wait until the cat condition variable is called.
		pthread_cond_wait(&cvC, &lock);
	} 
	
	// Increment cats playing and total cats who played.
	cats++;
	p_cats++;
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void dog_exit(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Decrement the amount of dogs playing
	dogs--;
	// Send the condition for Cats to wake
	// Cats will then check if there are cats playing
	pthread_cond_signal(&cvC);
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void dog_enter(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Check condition.
	// Ensure there are no cats playing.
	// Also check if the monitor has instructed the thread to quit.
	while (cats > 0 || quit == 1) {	
		// If the monitor has set this, exit the thread.
		if (quit == 1) {
			// Signal everyone to wake so they can quit.
			pthread_cond_signal(&cvC);
			pthread_cond_signal(&cvDB);

			// Release the lock
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
		}
		// Wait until the dog and bird condition variable is called.
		pthread_cond_wait(&cvDB, &lock);
	} 
	
	// Increment dogs playing and total cats who played.
	dogs++;
	p_dogs++;
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void bird_exit(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Decrement the amount of birds playing
	birds--;
	// Send the condition for Cats to wake
	// Cats will then check if there are cats playing
	pthread_cond_signal(&cvC);
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void bird_enter(void) {
	// Grab the lock
	pthread_mutex_lock(&lock);
	// Check condition.
	// Ensure there are no cats playing.
	// Also check if the monitor has instructed the thread to quit.
	while (cats > 0 || quit == 1) {	
		// If the monitor has set this, exit the thread.
		if (quit == 1) {
			// Signal everyone to wake so they can quit.
			pthread_cond_signal(&cvC);
			pthread_cond_signal(&cvDB);

			// Release the lock
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
		}
		// Wait until the dog and bird condition variable is called.
		pthread_cond_wait(&cvDB, &lock);
	} 
	
	// Increment birds playing and total cats who played.
	birds++;
	p_birds++;
	// Release the lock
	pthread_mutex_unlock(&lock);
}

void *startThread(void *arg) {
	// Grab struct for the thread
	thread_data_t *data = (thread_data_t *)arg;

	// Loop until instructed to quit
	while (quit == 0) {
		// Determine which animal this is
		switch (data->animalType) {
			case 0:
				// Set us up the cat
				cat_enter();
				// Play!
				play();
				// Done playing, call exit function.
				cat_exit();
				break;

			case 1:
				// Set us up the dog
				dog_enter();
				// Play!
				play();
				// Done playing, call exit function.
				dog_exit();
				break;

			case 2:
				// Set us up the bird
				bird_enter();
				// Play!
				play();
				// Done playing, call exit function.
				bird_exit();
				break;
		}
	}
	// When exiting, make sure to wake all threads so they can exit
	pthread_cond_signal(&cvC);
	pthread_cond_signal(&cvDB);

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	// Make sure three parameters are passed.
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
		printf("%s\n", "You must provide three parameters");
		return 1;
	}

	// Try to convert parameters to integers.
	n_cats = atoi(argv[1]);
	n_dogs = atoi(argv[2]);
	n_birds = atoi(argv[3]);

	// Make sure parameters parsed correctly
	// Both the character 0 and a parsing error end up being 0,
	// so we have to check and see if the user did pass in 0 or not.
	if ((!n_cats && !isdigit(*argv[1])) || (!n_dogs && !isdigit(*argv[2])) || (!n_birds && !isdigit(*argv[3]))) {
		printf("%s\n", "You must provide three valid integers");
		return 1;
	}
	// Make sure parameters are within the correct ranges
	if (n_cats < -1 || n_cats > 100 || n_dogs < -1 || n_dogs > 100 || n_birds < -1 || n_birds > 100) {
		printf("%s\n", "Integers must be between 0 and 99");
		return 1;
	}

	// Set up vars
	int i, rc, threads;
	cats = 0; dogs = 0; birds = 0;
	threads = n_cats + n_dogs + n_birds;
	pthread_t thr[threads];
	thread_data_t thr_data[threads];
	int t_cats = 0; int t_dogs = 0; int t_birds = 0;

	// Attempt to initialize lock and cvs
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("Failed to initialize mutex\n");
		return 1;
	}
	if (pthread_cond_init(&cvC, NULL) != 0 || pthread_cond_init(&cvDB, NULL) != 0) {
		printf("Failed to initialize mutex\n");
		pthread_mutex_destroy(&lock);
		return 1;
	}
	
	i = 0;
	// Loop through and set up thread structures with animal data.
	// Only attempt at fairness that I made.
	// Try to coalate the animal types so it isn't front-loaded with one type.
	while (i < threads) {
		if (t_cats < n_cats) {
			t_cats++;
			thr_data[i++].animalType = 0;
		}
		if (t_dogs < n_dogs) {
			t_dogs++;
			thr_data[i++].animalType = 1;
		}
		if (t_birds < n_birds) {
			t_birds++;
			thr_data[i++].animalType = 2;
		}
	}

	// Create the threads and let them run.
	for (i = 0; i < threads; i++) {
		if ((rc = pthread_create(&thr[i], NULL, startThread, &thr_data[i]))) {
			printf("error: pthread_create, rc: %d\n", rc);
	  		exit(EXIT_FAILURE);
		}
	}

	// Go to sleep for 10 seconds while the animals play.
	sleep(10);

	// Tell the threads to quit and wait for them to exit
	quit = 1;
	for (i = 0; i < threads; i++) {
		pthread_join(thr[i], NULL);
	}
	// Print out the play numbers
	printf("cat play = %d, dog play = %d, bird play = %d\n", p_cats, p_dogs, p_birds);

	// Clean up
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cvC);
	pthread_cond_destroy(&cvDB);

	return 0;
}