#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_FAILURE -1
#define STATUS_SUCCESS 0
#define STATUS_MALLOC_FAIL NULL
#define CREATORS_COUNT 4
#define NO_DETAILS 0
#define DETAIL_A_TIME_WEIGTH 1
#define DETAIL_B_TIME_WEIGTH 2
#define DETAIL_C_TIME_WEIGTH 3
#define WIDGETS_MAX 100
#define STOP 1

typedef struct widgetInfoContainer {
    int * detailsACount;
    int * detailsBCount;
    int * detailsCCount;
    int * modulesCount;
    int * widgetsCount;
} widgetInfoContainer;

int stop = 0;
char errorBuffer[256];

sem_t detA, detB, detC, module;

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
    widgetInfoContainer * info = (widgetInfoContainer*) args;
    int * detailsACount = info->detailsACount;

    while (stop != STOP){
        sleep(DETAIL_A_TIME_WEIGTH);

        verifyFunctionsByErrno(sem_wait(&detA), "sem_wait");
        *detailsACount = *detailsACount + 1;

        fprintf(stdout,"Detail A created! %d\n", *detailsACount);
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&detA), "sem_post");

    }

    return NULL;
}

void *detailBCreator(void *args) {
    widgetInfoContainer * info = (widgetInfoContainer*) args;
    int * detailsBCount = info->detailsBCount;

    while (stop != STOP){
        sleep(DETAIL_B_TIME_WEIGTH);

        verifyFunctionsByErrno(sem_wait(&detB), "sem_wait");
        *detailsBCount = *detailsBCount + 1;

        fprintf(stdout,"Detail B created! %d\n", *detailsBCount);
        fflush(stdout);
        verifyFunctionsByErrno(sem_post(&detB), "sem_post");

    }

    return NULL;
}

void *detailCCreator(void *args) {
    widgetInfoContainer * info = (widgetInfoContainer*) args;
    int * detailsCCount = info->detailsCCount;

    while (stop != STOP){
        sleep(DETAIL_C_TIME_WEIGTH);

        verifyFunctionsByErrno(sem_wait(&detC), "sem_wait");
        *detailsCCount = *detailsCCount + 1;

        fprintf(stdout,"Detail C created! %d\n", *detailsCCount);
        fflush(stdout);
        verifyFunctionsByErrno(sem_post(&detC), "sem_post");

    }

    return NULL;
}

void *moduleCreator(void *args){
    widgetInfoContainer * info = (widgetInfoContainer*) args;

    int * modulesCount = info->modulesCount;
    int * detailsACount = info->detailsACount;
    int * detailsBCount = info->detailsBCount;

    while (stop != STOP){

        verifyFunctionsByErrno(sem_wait(&detA), "sem_wait");
        verifyFunctionsByErrno(sem_wait(&detB), "sem_wait");

        if(*detailsACount == NO_DETAILS || *detailsBCount == NO_DETAILS){
            verifyFunctionsByErrno(sem_post(&detB), "sem_post");
            verifyFunctionsByErrno(sem_post(&detA), "sem_post");
            continue;
        }

        verifyFunctionsByErrno(sem_wait(&module), "sem_wait");
        *modulesCount = *modulesCount + 1;
        *detailsACount = *detailsACount - 1;
        *detailsBCount = *detailsBCount - 1;
        verifyFunctionsByErrno(sem_post(&module), "sem_post");

        fprintf(stdout,"Module created! %d. Left: %d details A, %d details B\n", *modulesCount, *detailsACount, *detailsBCount);
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&detB), "sem_post");
        verifyFunctionsByErrno(sem_post(&detA), "sem_post");

    }

    return NULL;
}

void *widgetCreator(void *args){

    widgetInfoContainer * info = (widgetInfoContainer*) args;

    int * modulesCount = info->modulesCount;
    int * detailsCCount = info->detailsACount;
    int * widgetsCount = info->widgetsCount;

    while (stop != STOP){

        verifyFunctionsByErrno(sem_wait(&detC), "sem_wait");
        verifyFunctionsByErrno(sem_wait(&module), "sem_wait");

        if(*detailsCCount == NO_DETAILS || *modulesCount == NO_DETAILS){
            verifyFunctionsByErrno(sem_post(&detC), "sem_post");
            verifyFunctionsByErrno(sem_post(&module), "sem_post");
            continue;
        }

        *widgetsCount = *widgetsCount + 1;
        *modulesCount = *modulesCount - 1;
        *detailsCCount = *detailsCCount - 1;

        if(*widgetsCount > WIDGETS_MAX){
            stop = STOP;
        }

        fprintf(stdout,"Widget created! %d. Left: %d modules, %d details C\n", *widgetsCount, *modulesCount, *detailsCCount);
        fflush(stdout);

        verifyFunctionsByErrno(sem_post(&module), "sem_post");
        verifyFunctionsByErrno(sem_post(&detC), "sem_post");

    }

    return NULL;
}

void initSemaphores(){
    verifyFunctionsByErrno(sem_init(&detA, 0, 1), "sem_init");
    verifyFunctionsByErrno(sem_init(&detB, 0, 1), "sem_init");
    verifyFunctionsByErrno(sem_init(&detC, 0, 1), "sem_init");
    verifyFunctionsByErrno(sem_init(&module, 0, 1), "sem_init");
}

int main() {

    pthread_t creators[CREATORS_COUNT];

    int *detailsACount = (int *) calloc(1, sizeof(int));
    int *detailsBCount = (int *) calloc(1, sizeof(int));
    int *detailsCCount = (int *) calloc(1, sizeof(int));
    int *modulesCount = (int *) calloc(1, sizeof(int));
    int *widgetsCount = (int *) calloc(1, sizeof(int));

    if(detailsACount == STATUS_MALLOC_FAIL || detailsBCount == STATUS_MALLOC_FAIL || detailsCCount == STATUS_MALLOC_FAIL || modulesCount == STATUS_MALLOC_FAIL || widgetsCount == STATUS_MALLOC_FAIL){
        verifyFunctionsByErrno(STATUS_FAILURE, "calloc");
    }

    initSemaphores();

    widgetInfoContainer info = {detailsACount, detailsBCount, detailsCCount, modulesCount, widgetsCount};

    verifyPthreadFunctions(pthread_create(&creators[0], NULL, detailACreator, (void*) &info), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[1], NULL, detailBCreator, (void*) &info), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[2], NULL, detailCCreator, (void*) &info), "pthread_create");
    verifyPthreadFunctions(pthread_create(&creators[3], NULL, moduleCreator, (void*) &info), "pthread_create");
    widgetCreator((void*) &info);

    for (int i = 0; i < CREATORS_COUNT; ++i) {
        verifyPthreadFunctions(pthread_join(creators[i], NULL), "pthread_join");
    }

}