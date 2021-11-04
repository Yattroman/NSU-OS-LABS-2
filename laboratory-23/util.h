#ifndef NSU_OS_LABS_2_UTIL_H
#define NSU_OS_LABS_2_UTIL_H

#include <semaphore.h>
#include <errno.h>
#include <pthread.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_SUCCESS 0

char errorBuffer[BUFFER_DEF_LENGTH];

sem_t semaphore;

void verifyFunctionsByErrno(int returnCode, const char *functionName) {
    strerror_r(errno, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

#endif
