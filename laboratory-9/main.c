#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define STATUS_SUCCESS 0
#define MINIMAL_REQUIRED_ARGS 2
#define MINIMAL_REQUIRED_THREADS 1
#define BUFFER_DEF_LENGTH 256
#define ITERATION_VOLUME 1000000
#define TRUE 1
#define ENABLED 1
#define DISABLED 0

char errorBuffer[BUFFER_DEF_LENGTH];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

int executionStatus = ENABLED;
int latestIteration = 0;

typedef struct compInfo {
    double* localSumm;
    int* rank;
    int threadsNumber;
} compInfo;

void verifyPthreadFunctions(int returnCode, const char* functionName){
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if(returnCode < STATUS_SUCCESS){
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void * getResultsPart(void * args){
    compInfo * info = (compInfo*) args;
    int currentIteration = 0;

    while(TRUE){
        for (int i = currentIteration * ITERATION_VOLUME + *info->rank; i < (currentIteration+1) * ITERATION_VOLUME; i += info->threadsNumber) {
            *info->localSumm += ((i % 2 == 0) ? (1.0) : (-1.0)) / (2.0 * i + 1.0);
        }
        verifyPthreadFunctions(pthread_barrier_wait(&barrier), "pthread_barrier_wait");

        verifyPthreadFunctions(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
        if(executionStatus == 0 && currentIteration == latestIteration){
            printf("%d ", currentIteration);
            verifyPthreadFunctions(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
            break;
        } else {
            currentIteration++;
            if(latestIteration < currentIteration){
                latestIteration = currentIteration;
            }
        }
        verifyPthreadFunctions(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
    }

    return NULL;
}

void stopExecution() {
    executionStatus = DISABLED;
}

double getCalculatedPi(int threadsNumber){
    pthread_t tid[threadsNumber];
    double pi = 0;

    double * lSumms = (double*) malloc(sizeof(double)*threadsNumber);
    int * rank = (int*) malloc(sizeof(int)*threadsNumber);
    compInfo * info = (compInfo*) malloc(sizeof(compInfo)*threadsNumber);

    if(info == NULL || lSumms == NULL || rank == NULL){
        perror("There are problems with allocating memory");
        pthread_exit(NULL);
    }

    for (int i = 0; i < threadsNumber ; ++i) {
        lSumms[i] = 0.0;

        info->localSumm = &lSumms[i];
        info->rank = &rank[i];
        info->threadsNumber = threadsNumber;

        verifyPthreadFunctions(pthread_create(&tid[i], NULL, getResultsPart, (void*) info), "pthread_create");
    }

    for (int i = 0; i < threadsNumber ; ++i) {
        verifyPthreadFunctions(pthread_join(tid[i], NULL), "pthread_join");
        pi += lSumms[i];
    }

    pi *= 4.0;

    return pi;
}

int main(int argc, char** argv) {

    if(argc < MINIMAL_REQUIRED_ARGS){
        fprintf(stderr, "Not enough arguments entered.\nusage: <progname> <threads_number>\n");
        pthread_exit(NULL);
    }

    long threadsNumber = atol(argv[1]);

    if(threadsNumber < MINIMAL_REQUIRED_THREADS){
        fprintf(stderr, "Invalid threads number. Minimal number: 1. Try again.");
        pthread_exit(NULL);
    }

    verifyPthreadFunctions(pthread_barrier_init(&barrier, NULL, threadsNumber), "pthread_barrier_init");

    signal(SIGINT, stopExecution);
    signal(SIGTERM, stopExecution);

    double pi = getCalculatedPi(threadsNumber);
    printf("pi done: %.15g \n", pi);

    verifyPthreadFunctions(pthread_barrier_destroy(&barrier), "pthread_barrier_destroy");
    verifyPthreadFunctions(pthread_mutex_destroy(&mutex), "pthread_mutex_destroy");

    pthread_exit(EXIT_SUCCESS);
}

