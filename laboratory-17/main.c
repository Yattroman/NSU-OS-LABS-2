#include <unistd.h>

#include "stringProcessing.h"
#include "list.h"

#define MAX_STRINGS_NUMBER 1000

int stopFlag = 0;

int getStringsIntoList(List *stringsList) {

    int status;
    int stringsEntered = 0;
    char *tempLine = NULL;

    do {
        fprintf(stdout, "%d: ", stringsEntered + 1);
        status = readString(&tempLine);
        if (status == STATUS_FAILURE) {
            fprintf(stderr, "readString. Problems with reading string.");
            return STATUS_FAILURE;
        }

        if (strcmp(tempLine, "") == STATUS_SUCCESS) {
            printf("current list status...\n");
            printList(stringsList);
            continue;
        }

        if (strcmp(tempLine, "stop!") == STATUS_SUCCESS) {
            printf("stopped!\n");
            stopFlag = 1;
            printList(stringsList);
            break;
        }

        Node *node = createNode(tempLine);
        if (node == STATUS_FAILURE_MEMORY) {
            fprintf(stderr, "getStringsIntoList. Problems with creating node.");
        }

        push(stringsList, node);
        stringsEntered++;
    } while (stringsEntered < MAX_STRINGS_NUMBER);

    return stringsEntered;
}

void *sortingThread(void *args) {
    List *list = (List *) args;

    while (!stopFlag) {
        sleep(5);
        sortList(list);
    }

    return STATUS_SUCCESS;
}

int main() {
    pthread_t childThread;

    List *stringsList = (List *) malloc(sizeof(List));
    initList(stringsList);

    verifyPthreadFunctions(pthread_create(&childThread, NULL, sortingThread, (void *) stringsList), "pthread_create");

    getStringsIntoList(stringsList);

    verifyPthreadFunctions(pthread_join(childThread, NULL), "pthread_join");
    freeList(stringsList);

    pthread_exit(STATUS_SUCCESS);
}