
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_SUCCESS 0

char errorBuffer[BUFFER_DEF_LENGTH];

pthread_cond_t condVar;

void freeResources() {

}

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void * writeStrings(void * arg){

    return NULL;
}

int main(){
    pthread_t childrenThread;

    verifyPthreadFunctions(pthread_create(&childrenThread, NULL, writeStrings, (void *) "Children message"), "pthread_create");

}