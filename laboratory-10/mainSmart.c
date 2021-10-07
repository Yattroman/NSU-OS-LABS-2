#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define PHILOSOPHERS_NUMBER 5
#define DELAY 1
#define MAX_FOOD 5

int foodEaten[PHILOSOPHERS_NUMBER];
int state[PHILOSOPHERS_NUMBER];
int philosophersIDs[PHILOSOPHERS_NUMBER];

pthread_t philosophers[PHILOSOPHERS_NUMBER];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVars[PHILOSOPHERS_NUMBER];

int getLeftNeighbour(int philosopherID){
    return (philosopherID + PHILOSOPHERS_NUMBER - 1) % PHILOSOPHERS_NUMBER;
}

int getRightNeighbour(int philosopherID){
    return (philosopherID + 1) % PHILOSOPHERS_NUMBER;
}

int getTime(int philosopherID){
    return DELAY * (MAX_FOOD - foodEaten[philosopherID] + 1);
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

    int philosopherID = *((int*) args);
    fprintf(stdout,"Philosopher %d sitting down to dinner.\n", philosopherID);
    fflush(stdout);

    while(foodEaten[philosopherID] < MAX_FOOD){
        fprintf(stdout, "Philosopher %d: is thinking.\n", philosopherID);
        fflush(stdout);
        think(getTime(philosopherID));

        getForks(philosopherID);

        foodEaten[philosopherID] += 1;
        fprintf(stdout, "Philosopher %d: get dish %d.\n", philosopherID, foodEaten[philosopherID]);
        fflush(stdout);
        eat(getTime(philosopherID));

        downForks(philosopherID);
    }

    return NULL;
}

int main (int argn, char **argv) {

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        state[i] = THINKING;
        foodEaten[i] = 0;
        philosophersIDs[i] = i;
        pthread_cond_init(&condVars[i], NULL);
        pthread_create(&philosophers[i], NULL, philosopher, (void*) &philosophersIDs[i] );
    }

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        pthread_join(philosophers[i], NULL);
    }

    pthread_exit(0);
}