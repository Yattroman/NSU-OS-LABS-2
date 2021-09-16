#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10
#define STATUS_SUCCESS 0
#define STDOUT 1

void * writeStrings(void* arg){

    const char * childMessage = {"Children text.\n"};

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        write(STDOUT, childMessage, strlen(childMessage));
//        printf("Children text: %d\n", i);
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
        exit(EXIT_FAILURE);
    }

    executionStatus = pthread_join(tid[0], NULL);

    if(executionStatus != STATUS_SUCCESS){
        char buffer[256];
        strerror_r(executionStatus, buffer, sizeof(buffer));
        fprintf(stderr,"There are problems with joining thread.. Certainly: %s", buffer);
        exit(EXIT_FAILURE);
    }

    const char * parentMessage = {"Parent text.\n"};
    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        write(STDOUT, parentMessage, strlen(parentMessage));
//        printf("Parent text: %d\n", i);
    }

    return EXIT_SUCCESS;
}
