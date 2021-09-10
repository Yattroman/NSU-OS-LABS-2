#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10000000
#define CLEANUP_POP_ARG 0
#define STATUS_SUCCES 0

void cleanupHandler(void *arg)
{
    printf("Called clean-up handler: %s\n", (char*) arg);
}

void * writeStrings(void* arg){

    pthread_cleanup_push(cleanupHandler, "Child thread handler");

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        pthread_testcancel();
        printf("Children text: %d\n", i);
        pthread_testcancel();
    }

    pthread_cleanup_pop(CLEANUP_POP_ARG);

    pthread_exit((void*) 1);
}

int main() {
    thread_t tid[THREADS_NUMBER];
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
