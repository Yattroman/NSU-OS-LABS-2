#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define STEPS_NUMBER 200000
#define STATUS_SUCCESS 0

typedef struct compInfo {
    int start;
    int end;
} compInfo;

void * getResultsPart(void * args){
    compInfo * info = (compInfo*) args;

    int start = info->start;
    int end = info->end;

    double * localPi = (double*) malloc(sizeof(double));
    *localPi = 0;

    for (int i = start; i < end ; ++i) {
        *localPi += 1.0/(i*4.0 + 1.0);
        *localPi -= 1.0/(i*4.0 + 3.0);
    }

    free(info);

    return localPi;
}

int main(int argc, char** argv) {

    if(argc < 2){
        fprintf(stderr, "Not enough arguments entered.\nusage: <progname> <threads_number>\n");
        pthread_exit(NULL);
    }

    long threadsNumber = atol(argv[1]);
    pthread_t tid[threadsNumber];

    double pi = 0;
    int executionStatus;
    int workWeight = STEPS_NUMBER/threadsNumber;
    printf("WorkWeight: %d", workWeight);

    for (int i = 0; i < threadsNumber ; ++i) {

        compInfo * info = (compInfo*) malloc(sizeof(compInfo));

        info->start = i*workWeight;
        info->end = (i+1)*workWeight - 1;

        executionStatus = pthread_create(&tid[i], NULL, getResultsPart, (void*) info);

        if(executionStatus != STATUS_SUCCESS){
            char buffer[256];
            strerror_r(executionStatus, buffer, sizeof(buffer));
            fprintf(stderr,"There are problems with creating thread. Certainly: %s", buffer);
            pthread_exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threadsNumber ; ++i) {
        void * piPart;
        executionStatus = pthread_join(tid[i], &piPart);

        if(executionStatus != STATUS_SUCCESS){
            char buffer[256];
            strerror_r(executionStatus, buffer, sizeof(buffer));
            fprintf(stderr,"There are problems with joining thread.. Certainly: %s", buffer);
            pthread_exit(EXIT_FAILURE);
        }

        pi = pi + *((double*) piPart);

        free(piPart);
    }

    pi = pi * 4.0;
    printf("pi done: %.15g \n", pi);
    pthread_exit(EXIT_SUCCESS);
}

