#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>

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
#define MESSAGES_NUMBER 20

sem_t emptyBuffLock, fullBuffLock;
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;

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

typedef struct Info {
    Queue *queue;
    int producerNumber;
} Info;

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

void nodeDestroy(NodeQ *nodeq){
    free(nodeq->msg);
    free(nodeq);
    nodeq = NULL;
}

// msqdestroy должна вызываться после того, как будет известно, что ни одна нить больше не попытается выполнять операции над очередью.
void queueDestroy(Queue *queue) {
    NodeQ* current = queue->front;
    while (current){
        NodeQ *temp = current;
        current = current->previous;
        nodeDestroy(temp);
    }
    free(queue);
    queue = NULL;
}

// msgdrop unlock waiting operations get and put
void msqdrop(Queue *queue) {
    sem_post(&fullBuffLock);
    sem_post(&emptyBuffLock);
}

// msgput принимает в качестве параметра ASCIIZ строку символов, обрезает ее до 80 символов (если это необходимо) и помещает ее в очередь.
// if queue is full (10 strings) - msgput locks
// funtion return count of send symbols
int msgput(Queue *queue, char *msg) {
    if(queue == NULL){
        return STATUS_FAILURE;
    }

    char * msgToNode = NULL;

    msgToNode = (char*) malloc(sizeof(char)*MAX_STRING_LENGTH + 1);
    if(msgToNode == STATUS_MALLOC_FAIL){
        return STATUS_FAILURE;
    }
    memcpy(msgToNode, msg, sizeof(char)*MAX_STRING_LENGTH);
    msgToNode[MAX_STRING_LENGTH] = ENDLINE_CHARACTER;

    NodeQ *nodeq = createNodeQ(msgToNode);

    if(nodeq == STATUS_FAILURE_MEM){
        return STATUS_FAILURE;
    }

    sem_wait(&emptyBuffLock);

    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");
    if(!isQueueFull(queue)){
        if(isQueueEmpty(queue)){
            verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");
        }
        if(queue->front == NULL && queue->rear == NULL){
            queue->rear = nodeq;
            queue->front = nodeq;
            queue->cursize = 1;
        }

        queue->rear->previous = nodeq;
        queue->rear = nodeq;

        queue->cursize += 1;
    }
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");

    sem_post(&fullBuffLock);

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

    sem_wait(&fullBuffLock);

    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");
    if(!isQueueEmpty(queue)){
        if(isQueueFull(queue)){
            verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");
        }
        nodeq = queue->front;
        queue->front = nodeq->previous;
        nodeq->previous = NULL;

        queue->cursize -= 1;
    }
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");

    sem_post(&emptyBuffLock);

    char * msgToBuffer = NULL;
    char *msg = nodeq->msg;

    // clear buffer
    memset(buf, '\0', bufsize);
    memcpy(buf, msg, sizeof(char)*bufsize);

    if(strlen(msg) > bufsize){
        msgToBuffer[bufsize-1] = ENDLINE_CHARACTER;
    }

    return strlen(buf);
}

void initSemaphores(){
    sem_init(&emptyBuffLock, 0, 1);
    sem_init(&fullBuffLock, 0, 0);
}

void * consumer(void* args){
    Queue *queue = (Queue*) args;

    for (int i = 0; i < MESSAGES_NUMBER; ++i) {
        char tempStr[100];
        msgget(queue, tempStr, 100);
        printf("%s", tempStr);
    }

    return NULL;
}

void * producer(void* args){
    Info *info = (Info*) args;
    Queue *queue = info->queue;

    char msgPrefix[30];
    sprintf(msgPrefix, "Producer %d Message: #", info->producerNumber);

    for (int i = 0; i < MESSAGES_NUMBER; ++i) {
        char numberBuffer[20];
        char tempStr[100];
        sprintf(numberBuffer, "%d\n", i);
        strcpy(tempStr, msgPrefix);
        strcat(tempStr, numberBuffer);
//        printf("%s", tempStr);
        msgput(queue, tempStr);
    }

}

int main() {
    pthread_t consumers[3];
    pthread_t producers[3];

    Queue * queue = (Queue*) malloc(sizeof(Queue));
    if(queue == STATUS_MALLOC_FAIL){
        verifyFunctionsByErrno(STATUS_FAILURE, "malloc");
    }
    queueInit(queue);

    Info infoProducer1 = {queue, 1};
    Info infoProducer2 = {queue, 2};
    Info infoProducer3 = {queue, 3};

    verifyPthreadFunctions(pthread_create(&producers[0], NULL, producer, (void*) &infoProducer1), "pthread_create");
//    verifyPthreadFunctions(pthread_create(&producers[1], NULL, producer, (void*) &infoProducer2), "pthread_create");
//    verifyPthreadFunctions(pthread_create(&producers[2], NULL, producer, (void*) &infoProducer3), "pthread_create");
    verifyPthreadFunctions(pthread_create(&consumers[0], NULL, consumer, (void*) queue), "pthread_create");
//    verifyPthreadFunctions(pthread_create(&consumers[1], NULL, consumer, (void*) queue), "pthread_create");
    pthread_join(producers[0], NULL);

    /*char* buffer = (char*) malloc(sizeof(char)*MAX_STRING_LENGTH + 1);
    int st;

    for (int i = 0; i < MAX_STRING_LENGTH; ++i) {
        st = msgget(queue, buffer, MAX_STRING_LENGTH);
        fprintf(stdout, "%d, %s", st, buffer);
        fflush(stdout);
    }*/

//    queueDestroy(queue);

    pthread_exit(EXIT_SUCCESS);
}
