#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_FAILURE -1
#define STATUS_SUCCESS 0
#define CREATORS_COUNT 5
#define NO_DETAILS 0
#define DETAIL_A_TIME_WEIGTH 1
#define DETAIL_B_TIME_WEIGTH 2
#define DETAIL_C_TIME_WEIGTH 3
#define WIDGETS_MAX 100
#define STOP 1

int executionStatus = 0;
char errorBuffer[256];

sem_t detA, detB, detC, module, widget;

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void verifyFunctionsByErrno(int returnCode, const char *functionName) {
    strerror_r(errno, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

void *detailACreator(void *args) {
    while (executionStatus != STOP){
        sleep(DETAIL_A_TIME_WEIGTH);

        fprintf(stdout,"> Detail A created!\n");
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&detA), "sem_post");
    }

    return NULL;
}

void *detailBCreator(void *args) {
    while (executionStatus != STOP){
        sleep(DETAIL_B_TIME_WEIGTH);

        fprintf(stdout,"> Detail B created!\n");
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&detB), "sem_post");
    }

    return NULL;
}

void *detailCCreator(void *args) {
    while (executionStatus != STOP){
        sleep(DETAIL_C_TIME_WEIGTH);

        fprintf(stdout,"> Detail C created!\n");
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&detC), "sem_post");
    }

    return NULL;
}

void *moduleCreator(void *args){
    while (executionStatus != STOP){
        verifyFunctionsByErrno(sem_wait(&detA), "sem_wait");
        verifyFunctionsByErrno(sem_wait(&detB), "sem_wait");

        fprintf(stdout,"-> Module created!\n");
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&module), "sem_post");
    }

    return NULL;
}

void *widgetCreator(void *args){

    while (executionStatus != STOP){
        verifyFunctionsByErrno(sem_wait(&detC), "sem_wait");
        verifyFunctionsByErrno(sem_wait(&module), "sem_wait");

        fprintf(stdout,"--> Widget created!\n");
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&widget), "sem_post");
    }

    return NULL;
}

void initSemaphores(){
    verifyFunctionsByErrno(sem_init(&detA, 0, NO_DETAILS), "sem_init");
    verifyFunctionsByErrno(sem_init(&detB, 0, NO_DETAILS), "sem_init");
    verifyFunctionsByErrno(sem_init(&detC, 0, NO_DETAILS), "sem_init");
    verifyFunctionsByErrno(sem_init(&module, 0, NO_DETAILS), "sem_init");
    verifyFunctionsByErrno(sem_init(&widget, 0, NO_DETAILS), "sem_init");
}

void stopExecution() {
    executionStatus = STOP;
    fprintf(stdout, "Preparing to stop...\n");
    fflush(stdout);
}

void freeResources(){
    sem_destroy(&detA);
    sem_destroy(&detB);
    sem_destroy(&detC);
    sem_destroy(&module);
    sem_destroy(&widget);
}

int main() {

    pthread_t creators[CREATORS_COUNT];

    initSemaphores();

    signal(SIGINT, stopExecution);
    signal(SIGTERM, stopExecution);

    verifyPthreadFunctions(pthread_create(&creators[0], NULL, detailACreator, NULL), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[1], NULL, detailBCreator, NULL), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[2], NULL, detailCCreator, NULL), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[3], NULL, moduleCreator, NULL), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[4], NULL, widgetCreator, NULL), "pthread_create");

    for (int i = 0; i < CREATORS_COUNT; ++i) {
        verifyPthreadFunctions(pthread_join(creators[i], NULL), "pthread_join");
    }

    int valueA;
    int valueB;
    int valueC;
    int valueModule;
    int valueWidget;

    verifyFunctionsByErrno(sem_getvalue(&detA, &valueA), "sem_getvalue");
    verifyFunctionsByErrno(sem_getvalue(&detB, &valueB), "sem_getvalue");
    verifyFunctionsByErrno(sem_getvalue(&detC, &valueC), "sem_getvalue");
    verifyFunctionsByErrno(sem_getvalue(&module, &valueModule), "sem_getvalue");
    verifyFunctionsByErrno(sem_getvalue(&widget, &valueWidget), "sem_getvalue");

    fprintf(stdout, "Stopped!\n[%d Widgets, %d Modules, %d details A, %d details B, %d details C]\n",
            valueWidget, valueModule, valueA, valueB, valueC);
    fflush(stdout);

    freeResources();

    pthread_exit(EXIT_SUCCESS);
}