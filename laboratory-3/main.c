#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 4
#define STRINGS_NUMBER 3
#define MAX_STRING_LENGTH 100

char** initStringsForPrinting(char ** strings, int threadNumber){
    strings[THREADS_NUMBER][MAX_STRING_LENGTH];
    for (int i = 0; i < THREADS_NUMBER; ++i) {
        strings[i] = strncat("I'm strings printed by thread N", (const char *) threadNumber, 1);
    }

    return strings;
}

void * writeStrings(void* arg){
    char** stringsForPrint = (char**) arg;

    printf("Children text: %s\n", arg);

    return NULL;
}

int main() {
    thread_t tid[THREADS_NUMBER];
    int executionStatus;

    for (int i = 0; i < THREADS_NUMBER; ++i) {

    }

    for (int i = 0; i < THREADS_NUMBER; ++i) {
        executionStatus = pthread_create(&tid[i], NULL, writeStrings, (void*) i);

        if(executionStatus != 0){
            perror("There are problems with creating thread.");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS_NUMBER; ++i) {
        executionStatus = pthread_join(tid[i], NULL);

        if(executionStatus != 0){
            perror("There are problems with joining thread.");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
