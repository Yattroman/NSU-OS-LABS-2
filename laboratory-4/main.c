#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10000000
#define STATUS_SUCCES 0

void * writeStrings(void* arg){

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        printf("Children text: %d\n", i);
    }

    return NULL;
}

int main() {
    pthread_t tid[THREADS_NUMBER];
    int executionStatus = pthread_create(&tid[0], NULL, writeStrings, NULL);

    if(executionStatus != STATUS_SUCCES){
        perror("There are problems with creating thread.");
        exit(EXIT_FAILURE);
    }

    sleep(2);

    executionStatus = pthread_cancel(tid[0]);

    if(executionStatus != STATUS_SUCCES){
        perror("There are problems with canceling thread.");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
