#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <thread_db.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NUMBER 4
#define STRINGS_NUMBER 3
#define MAX_STRING_LENGTH 100

char** fillStringsForPrinting(char** strings, int threadNumber){
    const char numbers[] = {'1', '2', '3', '4'};

    for (char i = 0; i < THREADS_NUMBER; ++i) {
        memset(strings[i], '\0', sizeof(char)*MAX_STRING_LENGTH);
        strcpy(strings[i], strcat("I'm strings printed by thread N", &numbers[threadNumber]));
    }

    return strings;
}

void * writeStrings(void* arg){
    char** stringsForPrint = (char**) arg;

    for (int i = 0; i < THREADS_NUMBER; ++i) {
        printf("Children text: %s\n", stringsForPrint[i]);
    }

    return NULL;
}

int main() {
    pthread_t tid[THREADS_NUMBER];
    int executionStatus;

    char strings[THREADS_NUMBER][MAX_STRING_LENGTH];

    for (int i = 0; i < THREADS_NUMBER; ++i) {
        printf(strings)
    }

    /*for (int i = 0; i < THREADS_NUMBER; ++i) {
        executionStatus = pthread_create(&tid[i], NULL, writeStrings, (void*) getStringsForPrinting( (char **) strings, i));

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
    }*/

    return EXIT_SUCCESS;
}
