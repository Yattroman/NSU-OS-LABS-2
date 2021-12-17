#include "clientPart.h"
#include <pthread.h>
#include <unistd.h>

typedef struct statusFlags {
    int isEOF;
    int isBuffEmpty;
    int isBuffFull;
} StatusFl;

typedef struct BufferInfo {
    char *buffer;
    size_t lowBoundary;
    size_t highBoundary;
} BuffInfo;

typedef struct Arguments {
    int socket;
    StatusFl *flStatus;
    BuffInfo *buffInfo;
} Args;

pthread_mutex_t buffLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffCV = PTHREAD_COND_INITIALIZER;

// Добавить обёртку на pthread functions

void *readerRoutine(void *args) {
    Args *rArgs = (Args *) args;
    int socket = rArgs->socket;
    StatusFl *fl = rArgs->flStatus;
    BuffInfo *buffInfo = rArgs->buffInfo;

    pthread_mutex_lock(&buffLock);
    while (!fl->isEOF) {
        while (!fl->isBuffFull) {
            pthread_cond_wait(&buffCV, &buffLock);
        }
        int isHighBdrBiggerOrEqualThanLow = buffInfo->highBoundary >= buffInfo->lowBoundary;
        size_t availableSpaceToRead = (isHighBdrBiggerOrEqualThanLow) ? BIG_BUFFER_SIZE - buffInfo->highBoundary :
                                      buffInfo->lowBoundary - buffInfo->highBoundary;
        pthread_mutex_unlock(&buffLock);

        size_t readSymbolsNumber = read(socket, buffInfo->buffer + buffInfo->highBoundary, availableSpaceToRead);

        pthread_mutex_lock(&buffLock);
        if (readSymbolsNumber == STATUS_FAILURE) {
            // Обработать ошибкуs
        }
        buffInfo->highBoundary += readSymbolsNumber;
        if (readSymbolsNumber == NO_SYMBOLS) {
            fl->isEOF = YES;
            break;
        }
        if(buffInfo->highBoundary == BIG_BUFFER_SIZE){
            buffInfo->highBoundary = 0;
        }
        if (buffInfo->highBoundary == buffInfo->lowBoundary) {
            fl->isBuffFull = YES;
        }
        fl->isBuffEmpty = NO;

        pthread_cond_signal(&buffCV);
    }
    pthread_mutex_unlock(&buffLock);
    return STATUS_SUCCESS;
}

void *writerRoutine(void *args) {
    Args *rArgs = (Args *) args;
    StatusFl *fl = rArgs->flStatus;
    BuffInfo *buffInfo = rArgs->buffInfo;
    int writtenLines;

    pthread_mutex_lock(&buffLock);
    while(!(fl->isBuffEmpty && fl->isEOF)){
        while (!fl->isBuffEmpty) {
            pthread_cond_wait(&buffCV, &buffLock);
        }
        int isHighBdrBiggerThanLow = buffInfo->highBoundary > buffInfo->lowBoundary;

        size_t curPos = buffInfo->lowBoundary;
        size_t endPos = (isHighBdrBiggerThanLow) ? buffInfo->highBoundary : BIG_BUFFER_SIZE;

        for (; curPos < endPos && buffInfo->buffer[curPos] != '\n'; ++curPos);
        if(curPos < endPos){
            ++curPos;
            ++writtenLines;
        }
        pthread_mutex_unlock(&buffLock);

        if(writtenLines > MAX_LINES){
            char tempBuffer[VLITTLE_BUFFER_SIZE];
            read(STDIN, tempBuffer, VLITTLE_BUFFER_SIZE);
            writtenLines = 0;
        }

        size_t writtenSymbolsNumber = write(STDOUT, buffInfo->buffer + buffInfo->lowBoundary, curPos - buffInfo->lowBoundary);

        pthread_mutex_lock(&buffLock);
        buffInfo->lowBoundary += writtenSymbolsNumber;

        if (writtenSymbolsNumber == NO_SYMBOLS) {
            fl->isEOF = YES;
            break;
        }
        if(buffInfo->lowBoundary == BIG_BUFFER_SIZE){
            buffInfo->lowBoundary = 0;
        }
        if (buffInfo->highBoundary == buffInfo->lowBoundary) {
            fl->isBuffEmpty = YES;
        }
        fl->isBuffFull = NO;

        pthread_cond_signal(&buffCV);
    }
    pthread_mutex_unlock(&buffLock);
    return STATUS_SUCCESS;
}

int threadsClient(char *url, int port) {
    struct addrinfo *serverinfo;

    pthread_t writer;
    pthread_t reader;

    char textBuffer[BIG_BUFFER_SIZE];
    char getRequestBuffer[MED_BUFFER_SIZE];

    int socket = openSocket(url, port, &serverinfo);
    if (socket == STATUS_FAILURE) {
        fprintf(stderr, "ебизи");
    }

    char *message = prepareGetRequest(getRequestBuffer, MED_BUFFER_SIZE, url);
    send(socket, message, strlen(message), 0);

    StatusFl fl = {0, 1, 0};
    BuffInfo info = {textBuffer, 0 ,0};
    Args args = {socket, &fl, &info};

    pthread_create(&writer, NULL, writerRoutine, (void*) &args);
    pthread_create(&reader, NULL, readerRoutine, (void*) &args);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    freeaddrinfo(serverinfo);
}

