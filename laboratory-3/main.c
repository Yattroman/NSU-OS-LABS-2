#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREADS_NUMBER 4
#define STRINGS_NUMBER 3

void *writeStrings(void *arg) {
    const char **stringsForPrint = (const char **) arg;

    for (int i = 0; i < STRINGS_NUMBER; ++i) {
        write(STDOUT_FILENO, stringsForPrint[i], strlen((const char *) stringsForPrint[i]));
    }

    return NULL;
}

int main() {
    pthread_t tid[THREADS_NUMBER];
    int executionStatus;
    char *strings[THREADS_NUMBER][STRINGS_NUMBER] = {{"i'm (1) ", "first (1) ",  "thread (1)\n"},
                                                     {"i'm (2) ", "second (2) ", "thread (2)\n"},
                                                     {"i'm (3) ", "third (3) ",  "thread (3)\n"},
                                                     {"i'm (4) ", "fourth (4) ", "thread (4)\n"}};

    for (int i = 0; i < THREADS_NUMBER; ++i) {
        executionStatus = pthread_create(&tid[i], NULL, writeStrings, (void *) &strings[i]);

        if (executionStatus != 0) {
            char buffer[256];
            strerror_r(executionStatus, buffer, sizeof(buffer));
            fprintf(stderr, "There are problems with creating thread. Certainly: %s", buffer);
            exit(EXIT_FAILURE);
        }

        executionStatus = pthread_join(tid[i], NULL);

        if(executionStatus != 0){
            perror("There are problems with joining thread.");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
