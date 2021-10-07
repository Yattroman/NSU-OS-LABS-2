#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 2
#define STRINGS_NUMBER 10
#define STATUS_SUCCESS 0
#define STDOUT 0
#define BUFFER_DEF_LENGTH 256

char errorBuffer[BUFFER_DEF_LENGTH];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void verifyPthreadFunctions(int returnCode, const char* functionName){
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if(returnCode < 0){
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void * writeStrings(void* arg){

    const char * childMessage = {"Children text.\n"};

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        pthread_mutex_lock(&mutex);
        write(STDOUT, childMessage, strlen(childMessage));
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t tid[THREADS_NUMBER];

    verifyPthreadFunctions(pthread_create(&tid[0], NULL, writeStrings, NULL), "pthread_create");

    const char * parentMessage = {"Parent text.\n"};

    for (int i = 1; i <= STRINGS_NUMBER; ++i) {
        pthread_mutex_lock(&mutex);
        write(STDOUT, parentMessage, strlen(parentMessage));
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(EXIT_SUCCESS);
}
