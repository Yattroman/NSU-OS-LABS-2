#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILOSOPHERS_NUMBER 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILOSOPHERS_NUMBER];
pthread_t philosophers[PHILOSOPHERS_NUMBER];

void *philosopher (void *args);
int foodOnTable ();
void getFork (int, int, char *);
void downForks (int, int);
pthread_mutex_t foodlock;

int sleepSeconds = 0;

void * philosopher (void *args) {
    int id = (int) args;
    printf ("Philosopher %d sitting down to dinner.\n", id);
    
    int rightFork = id;
    int leftFork = id + 1 % PHILOSOPHERS_NUMBER;
    int currentFoodNumber = foodOnTable();

    while (currentFoodNumber > 0) {

        /* Thanks to philosophers #1 who would like to
         * take a nap before picking up the forks, the other
         * philosophers may be able to eat their dishes and
         * not deadlock.
         */
        
        if (id == 1)
            sleep (sleepSeconds);

        printf("Philosopher %d: get dish %d.\n", id, currentFoodNumber);
        getFork(id, rightFork, "right");
        getFork(id, leftFork, "left ");

        printf("Philosopher %d: eating.\n", id);

        usleep(DELAY * (FOOD - currentFoodNumber + 1));

        downForks(leftFork, rightFork);

        currentFoodNumber = foodOnTable();
    }
    printf ("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int foodOnTable () {
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock (&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock (&foodlock);

    return myfood;
}

void getFork (int phil, int fork, char *hand) {
    pthread_mutex_lock (&forks[fork]);
    printf("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void downForks (int f1, int f2) {
    pthread_mutex_unlock (&forks[f1]);
    pthread_mutex_unlock (&forks[f2]);
}

int main (int argn, char **argv) {

    if (argn < 2) {
        fprintf(stderr, "Not enough arguments entered.\nusage: <progname> <sleep_seconds>\n");
    }

    sleepSeconds = atoi(argv[1]);

    pthread_mutex_init (&foodlock, NULL);
    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++)
        pthread_mutex_init (&forks[i], NULL);
    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++)
        pthread_create (&philosophers[i], NULL, philosopher, (void *) i );
    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++)
        pthread_join (philosophers[i], NULL);

    pthread_exit(0);
}