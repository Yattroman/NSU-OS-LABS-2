#include "stringProcessing.h"

#define INPUT_HOLDER_INIT_SIZE 256
#define EXPAND_COEFF 2;
#define FGETS_ERROR NULL
#define NEWLINE_CHARACTER '\n'
#define ENDSTRING_CHARACTER '\0'

int expandInputBuffer(char **inputHolder, size_t *bufferSize) {
    char *newInputHolder = NULL;
    int newInputHolderSize;

    if (*bufferSize == 0) {
        newInputHolderSize = INPUT_HOLDER_INIT_SIZE;
    } else {
        newInputHolderSize = *bufferSize * EXPAND_COEFF;
    }

    newInputHolder = realloc(*inputHolder, newInputHolderSize);

    if (newInputHolder == STATUS_FAILURE_MEMORY) {
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

    // Allocate memory of entered string size
    char *newInputHolder = realloc(tempLine, strlen(tempLine) + 1);

    if (newInputHolder == STATUS_FAILURE_MEMORY) {
        perror("expandInputBuffer. There are problems with realloc");
        return STATUS_FAILURE;
    }

    free(tempLine);

    tempLine = newInputHolder;
    *string = tempLine;

    return STATUS_SUCCESS;
}
