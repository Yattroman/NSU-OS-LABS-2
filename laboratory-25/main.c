#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define YES 1
#define NO 0
#define STATUS_FAILURE -1
#define STATUS_FAILURE_MEM NULL
#define STATUS_SUCCESS 0
#define BUFFER_DEF_LENGTH 256
#define EMPTY 0
#define STATUS_MALLOC_FAIL NULL
#define DEFAULT_QUEUE_CAPACITY 10
#define MAX_STRING_LENGTH 80
#define ENDLINE_CHARACTER '\0'

typedef struct NodeQ {
    char *msg;
    struct NodeQ *previous;
} NodeQ;

typedef struct Queue {
    NodeQ *front;
    NodeQ *rear;
    int cursize;
    int capacity;
} Queue;

char errorBuffer[256];

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

int isQueueEmpty(Queue* queue){
    return (queue->cursize == EMPTY) ? YES : NO;
}

int isQueueFull(Queue* queue){
    return (queue->cursize == queue->capacity) ? YES : NO;
}

NodeQ * createNodeQ(char* msg){
    NodeQ *node = (NodeQ*) malloc(sizeof(NodeQ));
    if(node == STATUS_MALLOC_FAIL){
        return STATUS_FAILURE_MEM;
    }

    node->msg = msg;
    node->previous = NULL;

    return node;
}

void queueInit(Queue *queue) {
    queue->capacity = DEFAULT_QUEUE_CAPACITY;
    queue->cursize = 0;
    queue->front = NULL;
    queue->rear = NULL;
}

// msqdestroy должна вызываться после того, как будет известно, что ни одна нить больше не попытается выполнять операции над очередью.
void queueDestroy(Queue *queue) {

}

// msgdrop unlock waiting operations get and put
void msqdrop(Queue *queue) {

}

// msgput принимает в качестве параметра ASCIIZ строку символов, обрезает ее до 80 символов (если это необходимо) и помещает ее в очередь.
// Если очередь содержит более 10 записей, msgput блокируется. Функция возвращает количество переданных символов.
int msgput(Queue *queue, char *msg) {
    if(queue == NULL){
        putc('l', stdout);
        return STATUS_FAILURE;
    }

    char * msgToNode = NULL;

    msgToNode = (char*) malloc(sizeof(char)*MAX_STRING_LENGTH + 1);
    if(msgToNode == STATUS_MALLOC_FAIL){
        putc('m', stdout);
        return STATUS_FAILURE;
    }
    memcpy(msgToNode, msg, sizeof(char)*MAX_STRING_LENGTH);
    msgToNode[MAX_STRING_LENGTH] = ENDLINE_CHARACTER;

    NodeQ *nodeq = createNodeQ(msgToNode);

    if(nodeq == STATUS_FAILURE_MEM){
        putc('a', stdout);
        return STATUS_FAILURE;
    }

    // It will be critical section
    if(!isQueueFull(queue)){
        if(queue->front == NULL && queue->rear == NULL){
            queue->rear = nodeq;
            queue->front = nodeq;
        }

        queue->rear->previous = nodeq;
        queue->rear = nodeq;

        queue->cursize += 1;
    }

    return strlen(msgToNode);
}

// msgget return first string from queue, trimmed to size of user buffer (if it necessary)
// if queue is empty - msgget locks
// funtion return count of read symbols
int msgget(Queue *queue, char *buf, size_t bufsize) {
    if(queue == NULL){
        return STATUS_FAILURE;
    }

    NodeQ *nodeq = NULL;

    // It will be critical section
    if(!isQueueEmpty(queue)){
        nodeq = queue->front;
        queue->front = nodeq->previous;
        nodeq->previous = NULL;

        queue->cursize -= 1;
    }

    char * msgToBuffer = NULL;
    char *msg = nodeq->msg;

    // clear buffer
    memset(buf, '\n', bufsize);
    memcpy(buf, msg, sizeof(char)*bufsize);

    if(strlen(msg) > bufsize){
        msgToBuffer[bufsize-1] = ENDLINE_CHARACTER;
    }

    return strlen(buf);
}

int main() {
    Queue * queue = (Queue*) malloc(sizeof(Queue));
    if(queue == STATUS_MALLOC_FAIL){
        verifyFunctionsByErrno(STATUS_FAILURE, "malloc");
    }

    queueInit(queue);

    msgput(queue, "hah");
    msgput(queue, "sdasas");
    msgput(queue, "hdfdfasd");
    msgput(queue, "hahasdsadsadfdfdfdsfdsf");

    char* buffer = (char*) malloc(sizeof(char)*MAX_STRING_LENGTH + 1);
    int st;

    for (int i = 0; i < 4; ++i) {
        st = msgget(queue, buffer, MAX_STRING_LENGTH);
        fprintf(stdout, "%d, %s\n", st, buffer);
        fflush(stdout);
    }

    fprintf(stdout, "cs");

    pthread_exit(EXIT_SUCCESS);
}