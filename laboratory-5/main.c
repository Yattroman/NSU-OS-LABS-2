#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10000000
#define CLEANUP_POP_ARG 0
#define STATUS_SUCCESS 0

void cleanupHandler(void *arg)
{
    printf("Called clean-up handler: %s\n", (char*) arg);
}

void * writeStrings(void* arg){

    pthread_cleanup_push(cleanupHandler, "Child thread handler");

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        printf("Children text: %d\n", i);
    }

    pthread_cleanup_pop(CLEANUP_POP_ARG);

    pthread_exit((void*) 1);
}

int main() {
    thread_t tid[THREADS_NUMBER];
    int executionStatus = pthread_create(&tid[0], NULL, writeStrings, NULL);

    if(executionStatus != STATUS_SUCCESS){
        char buffer[256];
        strerror_r(executionStatus, buffer, sizeof(buffer));
        fprintf(stderr,"There are problems with creating thread. Certainly: %s", buffer);
        pthread_exit(NULL);
    }

    sleep(2);

    executionStatus = pthread_cancel(tid[0]);

    if(executionStatus != STATUS_SUCCESS){
        char buffer[256];
        strerror_r(executionStatus, buffer, sizeof(buffer));
        fprintf(stderr,"There are problems with canceling thread. Certainly: %s", buffer);
        pthread_exit(NULL);
    }

    pthread_exit(EXIT_SUCCESS);
}
