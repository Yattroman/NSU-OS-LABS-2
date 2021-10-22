#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define PHILOSOPHERS_NUMBER 5
#define DELAY 0
#define MAX_FOOD 51
#define BUFFER_DEF_LENGTH 256

char errorBuffer[BUFFER_DEF_LENGTH];
int foodEaten[PHILOSOPHERS_NUMBER];
int state[PHILOSOPHERS_NUMBER];
int philosophersIDs[PHILOSOPHERS_NUMBER];

int food = MAX_FOOD;

pthread_t philosophers[PHILOSOPHERS_NUMBER];
pthread_mutex_t forks[PHILOSOPHERS_NUMBER];

pthread_cond_t forksCV = PTHREAD_COND_INITIALIZER;

pthread_mutex_t forkslock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t foodlock = PTHREAD_MUTEX_INITIALIZER;

void freeResources(){

}

void verifyPthreadFunctions(int returnCode, const char* functionName){
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if(returnCode < 0){
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        freeResources();
        pthread_exit(NULL);
    }
}

int getLeftForkNumber(int philosopherID){
    return (philosopherID + PHILOSOPHERS_NUMBER - 1) % PHILOSOPHERS_NUMBER;
}

int getRightForkNumber(int philosopherID){
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

int tryGetFork(int philosopherID, char * hand, int fork){
    printf ("Philosopher %d: trying to get fork %d in %s hand\n", philosopherID, fork, hand);
    return (pthread_mutex_trylock(&forks[fork]) == 0);
}

void downFork(int philosopherID, char * hand, int fork){
    pthread_mutex_unlock(&forks[fork]);
    printf("Philosopher %d: downs fork %d from %s hand\n", philosopherID, fork, hand);
}

void downForks(int philosopherID){
    downFork(philosopherID, "left", getLeftForkNumber(philosopherID));
    downFork(philosopherID, "right", getRightForkNumber(philosopherID));
}

int foodOnTable() {
    int myfood;

    pthread_mutex_lock (&foodlock);
    if (food > 0) {
        food--;
    }

    myfood = food;
    pthread_mutex_unlock (&foodlock);

    return myfood;
}

void returnFood(int philosopherID){
    pthread_mutex_lock(&foodlock);
    food++;
    pthread_mutex_unlock(&foodlock);
}

void * philosopher (void *args) {

    int philosopherID = *((int*) args);
    fprintf(stdout,"Philosopher %d sitting down to dinner.\n", philosopherID);
    fflush(stdout);

    while(foodOnTable()){

        pthread_mutex_lock(&forkslock);
        printf ("Philosopher %d: try get dish %d.\n", philosopherID, foodEaten[philosopherID]);

        if(!tryGetFork(philosopherID, "left", getLeftForkNumber(philosopherID))){
            returnFood(philosopherID);
            pthread_cond_wait(&forksCV, &forkslock);
            pthread_mutex_unlock(&forkslock);
        } else {
            printf ("Philosopher %d: got left fork %d\n", philosopherID, getLeftForkNumber(philosopherID));

            if(!tryGetFork(philosopherID, "right", getRightForkNumber(philosopherID))){
                downFork(philosopherID, "left", getLeftForkNumber(philosopherID));
                returnFood(philosopherID);
                pthread_cond_wait(&forksCV, &forkslock);
                pthread_mutex_unlock(&forkslock);
            } else {
                printf ("Philosopher %d: got right fork %d\n", philosopherID, getRightForkNumber(philosopherID));
                pthread_mutex_unlock(&forkslock);

                printf("Philosopher %d is eating: %d\n", philosopherID, foodEaten[philosopherID]);
                foodEaten[philosopherID]++;
                eat(getTime(philosopherID));

                pthread_mutex_lock(&forkslock);
                // down forks and notify others about this
                downForks(philosopherID);
                pthread_cond_broadcast(&forksCV);
                pthread_mutex_unlock(&forkslock);
            }

        }
        pthread_mutex_unlock(&forkslock);
    }

    return NULL;
}

int main (int argn, char **argv) {

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        foodEaten[i] = 0;
        philosophersIDs[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, (void*) &philosophersIDs[i] );
    }

    for (int i = 0; i < PHILOSOPHERS_NUMBER; i++){
        pthread_join(philosophers[i], NULL);
        printf("philosopher %d eat %d meals\n", i, foodEaten[i]);
    }

    freeResources();

    pthread_exit(0);
}