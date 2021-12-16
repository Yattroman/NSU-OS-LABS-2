#include <unistd.h>
#include "clientPart.h"

#define NO_DESC_READY 0
#define STDIN 0
#define STDOUT 1

// Input Buffer -> buffer which recv data from socket
// Output Buffer -> buffer which print data on screen

typedef struct fileDescInfo {
    fd_set writefds;
    fd_set readfds;
    int sckt;
} FDInfo;

typedef struct statusFlags {
    int isEOF;
    int isBuffEmpty;
    int isBuffFull;
} StatusFl;

typedef struct tBufferInfo {
    char *buffer;
    size_t lowBoundary;
    size_t highBoundary;
} BuffInfo;

int prepareFileDescSets(FDInfo *fdInf, StatusFl *fl, int printedLines) {
    FD_ZERO(&fdInf->writefds);
    FD_ZERO(&fdInf->readfds);

    // Adding filedesc in sets
    if (printedLines == MAX_LINES) {
        FD_SET(STDIN, &fdInf->readfds);
    }

    if (!fl->isEOF && !fl->isBuffFull) {
        FD_SET(fdInf->sckt, &fdInf->readfds);
    }

    if (printedLines < MAX_LINES && !fl->isBuffEmpty) {
        FD_SET(STDOUT, &fdInf->writefds);
    }
}

int handleSockedFD(FDInfo *fdInf, StatusFl *fl, BuffInfo *buffInfo) {
    if(FD_ISSET(fdInf->sckt, &fdInf->readfds) && !fl->isEOF && !fl->isBuffFull){
        int isHighBdrBiggerOrEqualThanLow = buffInfo->highBoundary >= buffInfo->lowBoundary;
        size_t availableSpaceToRead = (isHighBdrBiggerOrEqualThanLow) ? BIG_BUFFER_SIZE - buffInfo->highBoundary : buffInfo->lowBoundary - buffInfo->highBoundary;

        int readSymbolsNumber = read(fdInf->sckt, buffInfo->buffer + buffInfo->highBoundary, availableSpaceToRead);
        if(readSymbolsNumber == NO_SYMBOLS){
            fl->isEOF = YES;
            return STATUS_SUCCESS;
        }

        buffInfo->highBoundary += readSymbolsNumber;

        if(buffInfo->highBoundary == BIG_BUFFER_SIZE){
            buffInfo->highBoundary = 0;
        }
        if(buffInfo->highBoundary == buffInfo->lowBoundary){
            fl->isBuffFull = YES;
        }

        fl->isBuffEmpty = NO;
    }

    return STATUS_SUCCESS;
}

int handleStdoutFD(FDInfo *fdInf, StatusFl *fl, int *printedLines, BuffInfo *buffInfo) {
    if(FD_ISSET(STDOUT, &fdInf->writefds) && !fl->isBuffEmpty && *printedLines < MAX_LINES){
        int isHighBdrBiggerThanLow = buffInfo->highBoundary > buffInfo->lowBoundary;

        size_t curPos = buffInfo->lowBoundary;
        size_t endPos = (isHighBdrBiggerThanLow) ? buffInfo->highBoundary : BIG_BUFFER_SIZE;

        for (; curPos < endPos && buffInfo->buffer[curPos] != '\n'; ++curPos);

        if(curPos < endPos){
            ++curPos;
            ++(*printedLines);
        }

        int writtenSymbolsNumber = write(STDOUT, buffInfo->buffer + buffInfo->lowBoundary, curPos - buffInfo->lowBoundary);
        if(writtenSymbolsNumber == STATUS_FAILURE){
            perror("handleStdoutFD. write error");
        }

        buffInfo->lowBoundary += writtenSymbolsNumber;

        if(buffInfo->lowBoundary == BIG_BUFFER_SIZE){
            buffInfo->lowBoundary = 0;
        }
        if(buffInfo->highBoundary == buffInfo->lowBoundary){
            fl->isBuffEmpty = YES;
        }

        fl->isBuffFull = NO;
    }
}

int handleStdinFD(FDInfo *fdInf, StatusFl *fl, int *printedLines) {
    if (FD_ISSET(STDIN, &fdInf->readfds) && *printedLines == MAX_LINES) {
        char tempBuffer[VLITTLE_BUFFER_SIZE];
        read(STDIN, tempBuffer, VLITTLE_BUFFER_SIZE);
        *printedLines = 0;
    }
}

int isWorkDone(StatusFl *fl) {
    return (fl->isBuffEmpty && fl->isEOF) ? YES : NO;
}

int selectClient(char *url, int port) {
    int status;
    int printedLines = 0;
    char getRequestBuffer[MED_BUFFER_SIZE];
    struct addrinfo * serverinfo;
    int socket = openSocket(url, port, &serverinfo);

    char textBuffer[BIG_BUFFER_SIZE];

    if (socket < STATUS_SUCCESS) {
        perror( "selectClient. socket problem");
    }

    StatusFl fl = {0, 1, 0};
    FDInfo fdInf;
    fdInf.sckt = socket;
    BuffInfo buffInfo = {textBuffer, 0, 0};

    char *message = prepareGetRequest(getRequestBuffer, MED_BUFFER_SIZE, url);
    send(fdInf.sckt, message, strlen(message), 0);

    while (TRUE) {
        prepareFileDescSets(&fdInf, &fl, printedLines);
        status = select(fdInf.sckt + 1, &fdInf.readfds, &fdInf.writefds, NULL, NULL);
        if(status == STATUS_FAILURE){
            perror("selectClient. select problem");
        }

        // Checking for readiness of fds
        if (status == NO_DESC_READY) {
            continue;
        }

        handleSockedFD(&fdInf, &fl, &buffInfo);
        handleStdinFD(&fdInf, &fl, &printedLines);
        handleStdoutFD(&fdInf, &fl, &printedLines, &buffInfo);

        if (isWorkDone(&fl)) {
            break;
        }

    }

    freeaddrinfo(serverinfo);
}