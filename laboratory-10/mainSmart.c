#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define PHILOSOPHERS_NUMBER 5
#define DELAY 30000
#define MAX_FOOD 50

int foodEaten[PHILOSOPHERS_NUMBER];
int state[PHILOSOPHERS_NUMBER];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t philosophers[PHILOSOPHERS_NUMBER];
pthread_cond_t condVars[PHILOSOPHERS_NUMBER];

int getLeftNeighbour(int philosopherID){
    return (philosopherID + 1) % PHILOSOPHERS_NUMBER;
}

int getRightNeighbour(int philosopherID){
    return (philosopherID + PHILOSOPHERS_NUMBER - 1) % PHILOSOPHERS_NUMBER;
}

int getTime(int philosopherID){
    return DELAY * ( MAX_FOOD - foodEaten[philosopherID] + 1);
}

void think(int time) {
    sleep(time);
}

void eat(int time) {
    sleep(time);
}

void check(int philosopherID){
    if(state[getLeftNeighbour(philosopherID)] != EATING && state[philosopherID] == HUNGRY && state[getRightNeighbour(philosopherID)] != EATING){
        state[philosopherID] = EATING;
        pthread_cond_signal(&condVars[philosopherID]);
    }
}

void getForks(int philosopherID){
    pthread_mutex_lock(&mutex);
    state[philosopherID] = HUNGRY;
    check(philosopherID);
    while (state[philosopherID] != EATING) {
        pthread_cond_wait(&condVars[philosopherID], &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void downForks(int philosopherID){
    pthread_mutex_lock(&mutex);

    state[philosopherID] = THINKING;
    check(getLeftNeighbour(philosopherID));
    check(getRightNeighbour(philosopherID));

    pthread_mutex_unlock(&mutex);
}

void * philosopher (void *args) {

    int philosopherID = (int) args;
    printf ("Philosopher %d sitting down to dinner.\n", philosopherID);

    while(foodEaten[philosopherID] < MAX_FOOD){
        printf("Philosopher %d: is thinking.\n", philosopherID);
        think(getTime(philosopherID));

        getForks(philosopherID);

        foodEaten[philosopherID] += 1;
        printf("Philosopher %d: get dish %d.\n", philosopherID, foodEaten[philosopherID]);
        eat(getTime(philosopherID));

        downForks(philosopherID);
    }

    return NULL;
}

int main (int argn, char **argv) {

    if (argn < 2) {
        fprintf(stderr, "Not enough arguments entered.\nusage: <progname> <sleep_seconds>\n");
    }

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        state[i] = THINKING;
        foodEaten[i] = 0;
        pthread_cond_init(&condVars[i], NULL);
        pthread_create(&philosophers[i], NULL, philosopher, (void *) i );
    }

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        pthread_join(philosophers[i], NULL);
    }

    pthread_exit(0);
}