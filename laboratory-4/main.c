#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10000000
#define STATUS_SUCCESS 0
#define SLEEP_TIME_SEC 2

void * writeStrings(void* arg){

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        printf("Children text: %d\n", i);
    }

    return NULL;
}

int main() {
    pthread_t tid[THREADS_NUMBER];
    int executionStatus = pthread_create(&tid[0], NULL, writeStrings, NULL);

    if(executionStatus != STATUS_SUCCESS){
        char buffer[256];
        strerror_r(executionStatus, buffer, sizeof(buffer));
        fprintf(stderr,"There are problems with creating thread. Certainly: %s", buffer);
        pthread_exit(NULL);
    }

    sleep(SLEEP_TIME_SEC);

    executionStatus = pthread_cancel(tid[0]);

    if(executionStatus != STATUS_SUCCESS){
        char buffer[256];
        strerror_r(executionStatus, buffer, sizeof(buffer));
        fprintf(stderr,"There are problems with canceling thread. Certainly: %s", buffer);
        pthread_exit(NULL);
    }

    pthread_exit(EXIT_SUCCESS);
}
