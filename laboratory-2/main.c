#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10
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

    executionStatus = pthread_join(tid[0], NULL);

    if(executionStatus != STATUS_SUCCES){
        perror("There are problems with joining thread.");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        printf("Parent text: %d\n", i);
    }

    return EXIT_SUCCESS;
}
