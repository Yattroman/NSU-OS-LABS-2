#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>

#define STATUS_FAILURE -1
#define STATUS_SUCCESS 0
#define NO_ACTIONS 0
#define BUFFER_DEF_LENGTH 256

#define MIN_REQ_ARGS_NUMBER 3
#define DEF_PROTOCOL 0
#define BACKLOG  500
#define TIMEOUT  3000
#define MAX_THREADS 550
#define EMPTY_ADDR NULL
#define EMPTY_SOCKET -1
#define FD_NOT_READY 0

char errorBuffer[BUFFER_DEF_LENGTH];

int serverPort;
char* receiverIp;
int receiverPort;

int exitFlag = 0;

int threadsNumber = 0;
pthread_t *threads;

int verifyPthreadFunctions(int returnCode, const char *functionName, void (*freeResourcesPseudoSignal)()) {
    strerror_r(returnCode, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        if(freeResourcesPseudoSignal != NO_ACTIONS) { freeResourcesPseudoSignal(); }
        pthread_exit(NULL);
    }

    return returnCode;
}

int verifyFunctionsByErrno(int returnCode, const char *functionName, void (*freeResourcesPseudoSignal)()) {
    strerror_r(errno, errorBuffer, BUFFER_DEF_LENGTH);
    if (returnCode < STATUS_SUCCESS) {
        fprintf(stderr, "Error %s: %s\n", functionName, errorBuffer);
        if(freeResourcesPseudoSignal != NO_ACTIONS) { freeResourcesPseudoSignal(); }
        pthread_exit(NULL);
    }

    return returnCode;
}

void freeResources(int serverSocket, int clientSocket, int receiverSocket){
    if(serverSocket != EMPTY_SOCKET){
        close(serverSocket);
    }

    if(clientSocket != EMPTY_SOCKET){
        close(clientSocket);
    }

    if(receiverSocket != EMPTY_SOCKET){
        close(receiverSocket);
    }
}

void quitHandler(){
    exitFlag = 1;
}

void freeResourcesPseudoSignal(){
    quitHandler();
}

void initSignal(){
    static struct sigaction act;
    act.sa_handler = quitHandler;
    sigaction(SIGINT, &act, NULL);
}

void initArguments(int argc, char** argv){
    if(argc < MIN_REQ_ARGS_NUMBER){
        printf("not enough arguments entered, usage: <server_port> <receiver_ip> <receiver_port>\n");
        pthread_exit(NULL);
    }

    serverPort = atoi(argv[0]);
    memcpy(receiverIp, argv[1], sizeof(char)*(strlen(argv[1]+1)));
    receiverPort = atoi(argv[2]);

}

int transmit(int readSocket, int writeSocket){
    char buf[BUFFER_DEF_LENGTH];
    memset(buf, '\0', BUFFER_DEF_LENGTH);
    verifyFunctionsByErrno(read(readSocket, buf, BUFFER_DEF_LENGTH),"read", freeResourcesPseudoSignal);
    verifyFunctionsByErrno(write(writeSocket, buf, strlen(buf)), "write", freeResourcesPseudoSignal);
}

int initConnection(int *receiverSocket){
    *receiverSocket = verifyFunctionsByErrno(socket(AF_INET, SOCK_STREAM, DEF_PROTOCOL), "receiver socket open", freeResourcesPseudoSignal);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(receiverPort);
    address.sin_addr.s_addr = inet_addr(receiverIp);

    verifyFunctionsByErrno(connect(*receiverSocket, (struct sockaddr*) &address, sizeof(address)), "receiver socket connect", freeResourcesPseudoSignal);
    printf("[%d] successfully connected to %s:%d\n", *receiverSocket, receiverIp, receiverPort);

    return STATUS_SUCCESS;
}

void prepareSockets(int clientSocket, int receiverSocket, struct pollfd* fds){
    fds[0].fd = clientSocket;
    fds[1].fd = receiverSocket;
    fds[0].events = POLLIN | POLLHUP;
    fds[1].events = POLLIN | POLLHUP;
}

int spinConnection(int clientSocket, int receiverSocket) {
    printf("Connection spinning...");

    struct pollfd fds[2];
    prepareSockets(clientSocket, receiverSocket, fds);

    while(!exitFlag){
        verifyFunctionsByErrno(poll(fds, 2, TIMEOUT), "poll", freeResourcesPseudoSignal);
        printf("[%d/%d]: polled c/r: %d/%d\n", clientSocket, receiverSocket, fds[0].revents, fds[1].revents);

        if((fds[0].revents & POLLHUP) != 0 || (fds[1].revents & POLLHUP) != 0){
            printf("[%d/%d]: terminating session..", clientSocket, receiverSocket);
            fflush(stdout);
        }
        if(fds[0].revents != FD_NOT_READY){
            transmit(clientSocket, receiverSocket);
        }
        if(fds[1].revents != FD_NOT_READY){
            transmit(receiverSocket, clientSocket);
        }
    }

}

void *connectionBody(void* socket){
    int *clientSocket = (int*) socket;
    int receiverSocket = EMPTY_SOCKET;

    int status = initConnection(&receiverSocket);
    if(status == STATUS_SUCCESS){
        spinConnection(*clientSocket, receiverSocket);
    }

    free((int*)socket);
    freeResources(EMPTY_SOCKET, *clientSocket, receiverSocket);

    return NULL;
}

int initServer(int *serverSocket) {
    *serverSocket = verifyFunctionsByErrno(socket(AF_INET, SOCK_STREAM, DEF_PROTOCOL), "server socket open", freeResourcesPseudoSignal);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(serverPort);
    address.sin_addr.s_addr = INADDR_ANY;

    verifyFunctionsByErrno(bind(*serverSocket, (struct sockaddr*)&address, sizeof(address)), "server socket bind", freeResourcesPseudoSignal);
    verifyFunctionsByErrno(listen(*serverSocket, BACKLOG), "server socket listen", freeResourcesPseudoSignal);

    return STATUS_SUCCESS;
}

int spinServer(int serverFd){
    printf("Server spinning...");

    while(threadsNumber < MAX_THREADS && !exitFlag){
        int *clientSocket = (int*) malloc(sizeof(int));
        *clientSocket = verifyFunctionsByErrno(accept(serverFd, EMPTY_ADDR, EMPTY_ADDR), "server socket accept", freeResourcesPseudoSignal);
        verifyPthreadFunctions(pthread_create(&threads[threadsNumber], NULL, connectionBody, (void*) clientSocket), "pthread_create", freeResourcesPseudoSignal);
        threadsNumber++;
    }

    return STATUS_SUCCESS;
}

void joinServerThreads() {
    for (int i = 0; i < threadsNumber; ++i) {
        verifyPthreadFunctions(pthread_join(threads[i], NULL), "pthread_join", freeResourcesPseudoSignal);
    }
}

int main(int argc, char** argv){
    int serverSocket;

    initArguments(argc, argv);
    initSignal();

    if(initServer(&serverSocket)){
        spinServer(serverSocket);
    }

    freeResourcesPseudoSignal();
    joinServerThreads();
    freeResources(serverSocket, EMPTY_SOCKET, EMPTY_SOCKET);

    pthread_exit(NULL);
}

/*
 * Very Thanks to Andrey Rudometov (UltimateHikari)
 */
