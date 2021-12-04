#include "smartFunctions.h"

void *smartCalloc(size_t num, size_t size) {
    while (NOT_READY) {
        void *ptr = calloc(num, size);
        if (!IS_PTR_EMPTY(ptr)) {
            return ptr;
        }
        if (errno != EAGAIN) {
            fprintf(stderr, "smartCalloc. There are problems with memory allocation");
            return EMPTY;
        }
        sleep(RETRY_TIME_SEC);
    }
}

int smartOpenFile(char *path, int oflag, mode_t mode) {
    while (NOT_READY) {
        int fd = open(path, oflag, mode);
        if(fd != STATUS_FAILURE){
            return fd;
        }
        if(errno != EMFILE){
            fprintf(stderr, "smartOpenFile. There are problems with opening file");
            return STATUS_FAILURE;
        }
        sleep(RETRY_TIME_SEC);
    }
}

DIR *smartOpenDirectory(char *path) {
    while (NOT_READY) {
        DIR *dir = opendir(path);
        if (!IS_PTR_EMPTY(dir)) {
            return dir;
        }
        if (errno != EMFILE) {
            fprintf(stderr, "smartOpenDirectory. There are problems with opening directory");
            return EMPTY;
        }
        sleep(RETRY_TIME_SEC);
    }
}

int smartCreateThread(void *param, void* (*function)(void*)) {
    int status;
    pthread_t thread;
    while (NOT_READY) {
        status = pthread_create(&thread, NULL, function, param);
        if (status == STATUS_SUCCESS) {
            pthread_detach(thread);
            return STATUS_SUCCESS;
        }
        if (status != EAGAIN) {
            fprintf(stderr, "Unable to create thread");
            return STATUS_FAILURE;
        }
        sleep(RETRY_TIME_SEC);
    }
}