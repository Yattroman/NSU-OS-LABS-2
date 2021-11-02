#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_THREADS_NUMBER 100
#define STATUS_FAILURE -1
#define STATUS_SUCCESS 0
#define FGETS_ERROR NULL
#define ALLOCATION_MEM_ERROR NULL
#define NEWLINE_CHARACTER '\n'
#define ENDSTRING_CHARACTER '\0'
#define EXPAND_COEF 2
#define SLEEP_COEF 1000000
#define INPUT_HOLDER_INIT_SIZE 256
#define BUFFER_DEF_LENGTH 256

char errorBuffer[256];

void verifyPthreadFunctions(int returnCode, const char *functionName) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        pthread_exit(NULL);
    }
}

int expandInputBuffer(char **inputHolder, size_t *bufferSize) {
    char *newInputHolder = NULL;
    int newInputHolderSize;

    if (*bufferSize == 0) {
        newInputHolderSize = INPUT_HOLDER_INIT_SIZE;
    } else {
        newInputHolderSize = *bufferSize * EXPAND_COEF;
    }

    newInputHolder = realloc(*inputHolder, newInputHolderSize);

    if (newInputHolder == ALLOCATION_MEM_ERROR) {
        perror("expandInputBuffer. There are problems with realloc");
        return STATUS_FAILURE;
    }

    *inputHolder = newInputHolder;
    *bufferSize = newInputHolderSize;

    return STATUS_SUCCESS;
}

int readLine(char **line, size_t *currentLineLength) {
    char *inputHolder = NULL;
    char *fgetsStatus = NULL;

    size_t bufferSize = 0;
    size_t currentBufferPos = 0;

    do {
        if (bufferSize == 0 || inputHolder == NULL || currentBufferPos == bufferSize - 1) {
            int expandBufferStatus = expandInputBuffer(&inputHolder, &bufferSize);
            if (expandBufferStatus == STATUS_FAILURE) {
                return STATUS_FAILURE;
            }
        }
        fgetsStatus = fgets(&inputHolder[currentBufferPos], bufferSize - currentBufferPos, stdin);
        if (fgetsStatus == FGETS_ERROR) {
            return STATUS_FAILURE;
        }

        currentBufferPos += strlen(&inputHolder[currentBufferPos]);
    } while (inputHolder[currentBufferPos - 1] != NEWLINE_CHARACTER);

    *currentLineLength = currentBufferPos;
    *line = inputHolder;

    return STATUS_SUCCESS;
}

void truncateNewLineCharacter(char *line, size_t *lineLength) {
    if (*lineLength == 0 || line[*lineLength - 1] != NEWLINE_CHARACTER) {
        return;
    }
    line[*lineLength - 1] = ENDSTRING_CHARACTER;
    *lineLength -= 1;
}

int readString(char **string) {
    char *tempLine = NULL;
    size_t currentLineLength = 0;

    int readLineStatus = readLine(&tempLine, &currentLineLength);

    if (readLineStatus == STATUS_FAILURE) {
        fprintf(stderr, "readString. There are problems with reading line\n");
        return STATUS_FAILURE;
    }

    truncateNewLineCharacter(tempLine, &currentLineLength);
    *string = tempLine;

    return STATUS_SUCCESS;
}

int getStrings(char **strings) {
    int status;
    int stringsEntered = 0;
    char *tempLine = NULL;

    do {
        fprintf(stdout, "%d: ", stringsEntered + 1);
        status = readString(&tempLine);
        if (status == STATUS_FAILURE) {
            fprintf(stderr, "readString. There are problems with reading string.");
            return STATUS_FAILURE;
        }

        if (strcmp(tempLine, "stop!") == STATUS_SUCCESS) {
            printf("sorting started...\n");
            break;
        }

        char *stringHolder = (char *) malloc(strlen(tempLine));
        if (stringHolder == ALLOCATION_MEM_ERROR) {
            perror("There are problems with allocation memory.");
            return STATUS_FAILURE;
        }

        strcpy(stringHolder, tempLine);
        strings[stringsEntered] = stringHolder;

        stringsEntered++;
    } while (stringsEntered < MAX_THREADS_NUMBER);

    return stringsEntered;
}

void *printStringsSorted(void *arg) {
    char *string = (char*) arg;

    usleep(strlen(string)*SLEEP_COEF);

    printf("%s\n", string);

    return NULL;
}

int main() {
    pthread_t tid[MAX_THREADS_NUMBER];
    char *strings[MAX_THREADS_NUMBER];

    int stringsEntered = getStrings(strings);
    if (stringsEntered == STATUS_FAILURE) {
        fprintf(stderr, "There are problems with getting strings");
        pthread_exit(NULL);
    }

    for (int i = 0; i < stringsEntered; ++i) {
        verifyPthreadFunctions(pthread_create(&tid[i], NULL, printStringsSorted, (void*) strings[i]), "pthread_create");
    }

    for (int i = 0; i < stringsEntered; ++i) {
        verifyPthreadFunctions(pthread_join(tid[i], NULL), "pthread_join");
    }

    pthread_exit(NULL);
}