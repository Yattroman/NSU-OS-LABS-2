
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_SUCCESS 0
#define ITERATIONS_NUM 10

char errorBuffer[BUFFER_DEF_LENGTH];

pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int currentTurn;

typedef struct taskContext{
    int turnIdentifier;
    const char* infoString;
} taskContext;

void freeResources() {

    if (pthread_mutex_destroy(&mutex) != STATUS_SUCCESS) {
        fprintf(stderr, "pthread_mutex_destroy problems");
        pthread_exit(NULL);
    }
    if (pthread_cond_destroy(&condVar) != STATUS_SUCCESS) {
        fprintf(stderr, "pthread_cond_destroy problems");
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

void * writeStrings(void * arg){

    taskContext * info = (taskContext* ) arg;
    int turnIdentifier = info->turnIdentifier;
    const char * infoString = info->infoString;

    verifyPthreadFunctions(pthread_mutex_lock(&mutex), "pthread_mutex_lock");

    for (int i = 0; i < ITERATIONS_NUM; ++i) {
        while (currentTurn != turnIdentifier){
            verifyPthreadFunctions(pthread_cond_wait(&condVar, &mutex), "pthread_cond_wait");
        }

        fprintf(stdout, "%s: %d\n", infoString, i);
        fflush(stdout);

        currentTurn = !turnIdentifier;

        verifyPthreadFunctions(pthread_cond_signal(&condVar), "pthread_cond_signal");

    }

    verifyPthreadFunctions(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");

    return NULL;
}

int main(){
    pthread_t childrenThread;

    taskContext childrenTask = {0, "Child message"};
    taskContext parentTask = {1, "Parent message"};

    verifyPthreadFunctions(pthread_create(&childrenThread, NULL, writeStrings, (void *) &childrenTask), "pthread_create");
    writeStrings( (void*) &parentTask );
    void* returnedVal;
    verifyPthreadFunctions(pthread_join(childrenThread, &returnedVal), "pthread_join");

    if (returnedVal == PTHREAD_CANCELED) {
        printf("\nThread was cancelled\n");
    } else {
        printf("\nThread was joined normally\n");
    }

    freeResources();

    pthread_exit(STATUS_SUCCESS);
}