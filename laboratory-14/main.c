
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_SUCCESS 0
#define ITERATIONS_NUM 10

#define SEM_PARENT_START_VAL 1
#define SEM_CHILD_START_VAL 0

sem_t semaphoreFirst;
sem_t semaphoreSecond;

char errorBuffer[BUFFER_DEF_LENGTH];

void freeResources() {
    sem_destroy(&semaphoreFirst);
    sem_destroy(&semaphoreSecond);
}

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        freeResources();
        pthread_exit(NULL);
    }
}

void * writeStringsParent(void * arg){
    const char* message = (const char*) arg;

    for (int i = 0; i < ITERATIONS_NUM; ++i) {
        verifyPthreadFunctions(sem_wait(&semaphoreFirst), "sem_wait");
        fprintf(stdout, "%s: %d\n", message, i);
        fflush(stdout);
        verifyPthreadFunctions(sem_post(&semaphoreSecond), "sem_post");
    }

    return NULL;
}

void * writeStringsChild(void * arg){
    const char* message = (const char*) arg;

    for (int i = 0; i < ITERATIONS_NUM; ++i) {
        verifyPthreadFunctions(sem_wait(&semaphoreSecond), "sem_wait");
        fprintf(stdout, "%s: %d\n", message, i);
        fflush(stdout);
        verifyPthreadFunctions(sem_post(&semaphoreFirst), "sem_post");
    }

    return NULL;
}

void initSemaphores(){
    sem_init(&semaphoreFirst, 0 , SEM_PARENT_START_VAL);
    sem_init(&semaphoreSecond, 0 , SEM_CHILD_START_VAL);
}


int main(){
    pthread_t childrenThread;

    initSemaphores();

    verifyPthreadFunctions(pthread_create(&childrenThread, NULL, writeStringsChild, (void *) "Child message"), "pthread_create");
    writeStringsParent( (void*) "Parent message" );
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