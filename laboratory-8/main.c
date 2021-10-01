#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define STEPS_NUMBER 200000
#define STATUS_SUCCESS 0
#define BUFFER_DEF_LENGTH 256

char errorBuffer[BUFFER_DEF_LENGTH];

typedef struct compInfo {
    int start;
    int end;
} compInfo;

void verifyPthreadFunctions(int returnCode, const char* functionName){
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if(returnCode < 0){
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void * getResultsPart(void * args){
    compInfo * info = (compInfo*) args;

    int start = info->start;
    int end = info->end;

    double * localPi = (double*) malloc(sizeof(double));

    if(localPi == NULL){
        fprintf(stderr,"There are problems with allocating memory.");
        pthread_exit(NULL);
    }

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

    if(threadsNumber < 1){
        fprintf(stderr, "Invalid threads number. Minimal number: 1. Try again.");
        pthread_exit(NULL);
    }

    pthread_t tid[threadsNumber];

    double pi = 0;
    int workWeight = STEPS_NUMBER/threadsNumber;
    printf("WorkWeight: %d", workWeight);

    for (int i = 0; i < threadsNumber ; ++i) {
        compInfo * info = (compInfo*) malloc(sizeof(compInfo));
        info->start = i*workWeight;
        info->end = (i+1)*workWeight - 1;
        verifyPthreadFunctions(pthread_create(&tid[i], NULL, getResultsPart, (void*) info), "pthread_create");
    }

    for (int i = 0; i < threadsNumber ; ++i) {
        void * piPart;
        verifyPthreadFunctions(pthread_join(tid[i], &piPart), "pthread_join");
        pi = pi + *((double*) piPart);
        free(piPart);
    }

    pi = pi * 4.0;
    printf("pi done: %.15g \n", pi);
    pthread_exit(EXIT_SUCCESS);
}

