#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define YES 1
#define NO 0
#define STATUS_FAILURE (-1)
#define STATUS_FAILURE_MEM NULL
#define STATUS_SUCCESS 0
#define BUFFER_DEF_LENGTH 256
#define EMPTY 0
#define STATUS_MALLOC_FAIL NULL
#define DEFAULT_QUEUE_CAPACITY 10
#define MAX_STRING_LENGTH 80
#define ENDLINE_CHARACTER '\0'
#define MESSAGES_NUMBER 20
#define CONSUMERS_NUMBER 2
#define PRODUCERS_NUMBER 3
#define MSGS_TASK_NUMBER 40
#define TRUE 40
#define DROPPED_STATE 0

pthread_mutex_t msgLock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t notEmptyQueueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t notFullQueueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notEmptyQueueCV = PTHREAD_COND_INITIALIZER;
pthread_cond_t notFullQueueCV = PTHREAD_COND_INITIALIZER;

int msgsGot = 0;

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
    int number;
} Info;

char errorBuffer[256];
int isDropped = 0;

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

char *prepareMsgToNode(char *msg) {
    char *msgToNode = (char *) malloc(sizeof(char) * MAX_STRING_LENGTH + 1);
    if (msgToNode == STATUS_MALLOC_FAIL) {
        return STATUS_FAILURE_MEM;
    }
    memcpy(msgToNode, msg, sizeof(char) * MAX_STRING_LENGTH);
    msgToNode[MAX_STRING_LENGTH] = ENDLINE_CHARACTER;

    return msgToNode;
}

int isQueueEmpty(Queue *queue) {
    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");
    int result = (queue->cursize == EMPTY) ? YES : NO;
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");
    return result;
}

int isQueueFull(Queue *queue) {
    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");
    int result = (queue->cursize == queue->capacity) ? YES : NO;
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");
    return result;
}

NodeQ *createNodeQ(char *msg) {
    NodeQ *node = (NodeQ *) malloc(sizeof(NodeQ));
    if (node == STATUS_MALLOC_FAIL) {
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

void nodeDestroy(NodeQ *nodeq) {
    free(nodeq->msg);
    free(nodeq);
    nodeq = NULL;
}

void queueDestroy(Queue *queue) {
    NodeQ *current = queue->front;
    while (current) {
        NodeQ *temp = current;
        current = current->previous;
        nodeDestroy(temp);
    }
    free(queue);
    queue = NULL;
}

int areAllMsgsGot() {
    int result = (msgsGot == MSGS_TASK_NUMBER) ? YES : NO;
    return result;
}

void msgdrop() {
    isDropped = YES;
    verifyPthreadFunctions(pthread_cond_broadcast(&notEmptyQueueCV), "pthread_cond_broadcast");
    verifyPthreadFunctions(pthread_cond_broadcast(&notFullQueueCV), "pthread_cond_broadcast");
}

void incrementMsgsGot() {
    verifyPthreadFunctions(pthread_mutex_lock(&msgLock), "pthread_mutex_lock");
    if (areAllMsgsGot()) {
        msgdrop();
        verifyPthreadFunctions(pthread_mutex_unlock(&msgLock), "pthread_mutex_unlock");
        return;
    }
    msgsGot += 1;
    verifyPthreadFunctions(pthread_mutex_unlock(&msgLock), "pthread_mutex_unlock");
}

int msgput(Queue *queue, char *msg) {
    if(isDropped){
        return DROPPED_STATE;
    }

    char *msgToNode = prepareMsgToNode(msg);
    if (msgToNode == STATUS_FAILURE_MEM || queue == NULL) {
        return STATUS_FAILURE;
    }

    NodeQ *nodeq = createNodeQ(msgToNode);
    if (nodeq == STATUS_FAILURE_MEM) {
        return STATUS_FAILURE;
    }

    verifyPthreadFunctions(pthread_mutex_lock(&notFullQueueLock), "pthread_mutex_lock");
    if (isQueueFull(queue)) {
        fprintf(stdout, "Queue is full!\n");
        fflush(stdout);
        while (isQueueFull(queue) && !isDropped) {
            verifyPthreadFunctions(pthread_cond_wait(&notFullQueueCV, &notFullQueueLock), "pthread_cond_wait");
        }
    }
    if(isDropped){
        return DROPPED_STATE;
    }
    verifyPthreadFunctions(pthread_mutex_unlock(&notFullQueueLock), "pthread_mutex_unlock");

    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");

    if (queue->front == NULL) {
        queue->front = nodeq;
    }
    if (queue->rear == NULL) {
        queue->rear = nodeq;
    } else {
        queue->rear->previous = nodeq;
        queue->rear = nodeq;
    }

    queue->cursize += 1;

    // Notify that msg has been put into queue, and it isn't empty now
    verifyPthreadFunctions(pthread_cond_signal(&notEmptyQueueCV), "pthread_cond_broadcast");
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");

    return strlen(nodeq->msg);
}

int msgget(Queue *queue, char *buf, size_t bufsize) {
    if(isDropped){
        return DROPPED_STATE;
    }
    if (queue == NULL) {
        return STATUS_FAILURE;
    }

    NodeQ *nodeq = NULL;

    verifyPthreadFunctions(pthread_mutex_lock(&notEmptyQueueLock), "pthread_mutex_lock");
    if (isQueueEmpty(queue)) {
        fprintf(stdout, "Queue is empty!\n");
        fflush(stdout);
        while (isQueueEmpty(queue) && !isDropped) {
            verifyPthreadFunctions(pthread_cond_wait(&notEmptyQueueCV, &notEmptyQueueLock), "pthread_cond_wait");
        }
    }
    if(isDropped){
        return DROPPED_STATE;
    }
    verifyPthreadFunctions(pthread_mutex_unlock(&notEmptyQueueLock), "pthread_mutex_unlock");

    verifyPthreadFunctions(pthread_mutex_lock(&queueLock), "pthread_mutex_lock");

    nodeq = queue->front;
    queue->front = nodeq->previous;
    if (queue->front == NULL){
        queue->rear = NULL;
    }
    queue->cursize -= 1;

    // Notify that msg has been taken from queue, and it isn't full now
    verifyPthreadFunctions(pthread_cond_signal(&notFullQueueCV), "pthread_cond_broadcast");
    verifyPthreadFunctions(pthread_mutex_unlock(&queueLock), "pthread_mutex_unlock");

    // clear buffer
    memset(buf, '\0', bufsize);
    memcpy(buf, nodeq->msg, sizeof(char) * strlen(nodeq->msg));

    if (strlen(nodeq->msg) >= bufsize) {
        buf[bufsize - 1] = ENDLINE_CHARACTER;
    }

    nodeDestroy(nodeq);

    return strlen(buf);
}

void *consumer(void *args) {
    Info *info = (Info *) args;
    Queue *queue = info->queue;

    int status;
    char msgPrefix[30];
    sprintf(msgPrefix, "Consumer %d | ", info->number);

    int bufferSize = 100;
    char *buffer = (char *) malloc(sizeof(char) * bufferSize);

    int i = 1;

    do {
//        sleep(1);
        incrementMsgsGot();
        status = msgget(queue, buffer, bufferSize);
        if (status == STATUS_FAILURE) {
            fprintf(stdout, "[Error] msgget\n");
            fflush(stdout);
        } else if(status == DROPPED_STATE){
            break;
        }
        printf("i: %d | %s", i++, buffer);
    } while (TRUE);

    free(buffer);
    free(info);

    return NULL;
}

void *producer(void *args) {
    Info *info = (Info *) args;
    Queue *queue = info->queue;

    int status;
    char msgPrefix[30];
    sprintf(msgPrefix, "Producer %d Message: #", info->number);

    int i;
    for (i = 0; i < MESSAGES_NUMBER; ++i) {
        char numberBuffer[20];
        char tempStr[100];
//        sleep(1);
        sprintf(numberBuffer, "%d\n", i);
        strcpy(tempStr, msgPrefix);
        strcat(tempStr, numberBuffer);
        status = msgput(queue, tempStr);
        if (status == STATUS_FAILURE) {
            fprintf(stdout, "[Error] msgput!\n");
            fflush(stdout);
        } else if(status == DROPPED_STATE){
            break;
        }
    }

    free(info);

    return NULL;
}

Info *prepareInfoForExperiment(Queue *queue, int number) {
    Info *info = (Info *) malloc(sizeof(Info));

    info->queue = queue;
    info->number = number;

    return info;
}

void startExperiment(pthread_t *consumers, pthread_t *producers, Queue *queue) {
    Info *info1 = prepareInfoForExperiment(queue, 1);
    Info *info2 = prepareInfoForExperiment(queue, 2);

    verifyPthreadFunctions(pthread_create(&producers[0], NULL, producer, (void *) info1), "pthread_create");
    verifyPthreadFunctions(pthread_create(&producers[1], NULL, producer, (void *) info2), "pthread_create");
    verifyPthreadFunctions(pthread_create(&consumers[0], NULL, consumer, (void *) info1), "pthread_create");
    verifyPthreadFunctions(pthread_create(&consumers[1], NULL, consumer, (void *) info2), "pthread_create");
}

void endExperiment(pthread_t *consumers, pthread_t *producers) {
    verifyPthreadFunctions(pthread_join(producers[0], NULL), "pthread_join");
    verifyPthreadFunctions(pthread_join(producers[1], NULL), "pthread_join");
    verifyPthreadFunctions(pthread_join(consumers[0], NULL), "pthread_join");
    verifyPthreadFunctions(pthread_join(consumers[1], NULL), "pthread_join");
}

int main() {
    pthread_t consumers[CONSUMERS_NUMBER];
    pthread_t producers[PRODUCERS_NUMBER];

    Queue *queue = (Queue *) malloc(sizeof(Queue));
    if (queue == STATUS_MALLOC_FAIL) {
        verifyFunctionsByErrno(STATUS_FAILURE, "malloc");
    }

    queueInit(queue);
    startExperiment(consumers, producers, queue);
    endExperiment(consumers, producers);
    queueDestroy(queue);

    pthread_exit(EXIT_SUCCESS);
}
