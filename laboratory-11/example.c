#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 2
#define MUTEXES_NUMBER 3
#define ITERATIONS 10
#define STATUS_SUCCESS 0
#define STDOUT 0
#define YES 1
#define NO 0
#define BUFFER_DEF_LENGTH 256

char errorBuffer[BUFFER_DEF_LENGTH];
pthread_mutex_t mutexes[MUTEXES_NUMBER];

int hasItPrintedString = NO;

void freeResources(int mutexesCount) {
    for (int i = 0; i < mutexesCount; ++i) {
        if (pthread_mutex_destroy(&mutexes[i]) != STATUS_SUCCESS) {
            fprintf(stderr, "pthread_mutex_destroy problems");
            pthread_exit(NULL);
        }
    }
}

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        freeResources(MUTEXES_NUMBER);
        pthread_exit(NULL);
    }
}

void *writeStrings(void *str) {
    int currMutexIdx = 1;

    verifyPthreadFunctions(pthread_mutex_lock(&mutexes[2]), "pthread_mutex_lock");
    if (hasItPrintedString)
        verifyPthreadFunctions(pthread_mutex_unlock(&mutexes[0]), "pthread_mutex_unlock");
    for (int i = 0; i < ITERATIONS * MUTEXES_NUMBER; i++) {
        verifyPthreadFunctions(pthread_mutex_lock(&mutexes[currMutexIdx]), "pthread_mutex_lock");
        currMutexIdx = (currMutexIdx + 1) % MUTEXES_NUMBER;
        verifyPthreadFunctions(pthread_mutex_unlock(&mutexes[currMutexIdx]), "pthread_mutex_unlock");
        if (currMutexIdx == 2) {
            fprintf(stdout, "%s\n",(const char *) str);
            fflush(stdout);
            hasItPrintedString = YES;
        }
        currMutexIdx = (currMutexIdx + 1) % MUTEXES_NUMBER;
    }
    verifyPthreadFunctions(pthread_mutex_unlock(&mutexes[2]), "pthread_mutex_unlock");

    return NULL;
}

void initMutexes() {
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    verifyPthreadFunctions(pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK), "pthread_mutexattr_settype");
    for (int i = 0; i < MUTEXES_NUMBER; ++i) {
        if (pthread_mutex_init(&mutexes[i], &mattr) != STATUS_SUCCESS) {
            fprintf(stderr, "pthread_mutex_init problems, thread didn't created");
            freeResources(i + 1);
        }
    }
}

int main() {
    pthread_t childrenThread;

    initMutexes();

    verifyPthreadFunctions(pthread_create(&childrenThread, NULL, writeStrings, (void *) "Children message"), "pthread_create");

//    while (!printed) { sched_yield(); }

    writeStrings((void *) "Parent message");

    verifyPthreadFunctions(pthread_join(childrenThread, NULL), "pthread_join");

    pthread_exit(EXIT_SUCCESS);
}

/* Fundament by Dmitrii V Irtegov */