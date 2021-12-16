#include "networkPart.h"
#include "defAssigns.h"

int prepareServerAddrInfo(struct addrinfo ** serverinfo, int port, char *host){
    int status;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char portStr[LITTLE_BUFFER_SIZE];
    sprintf(portStr, "%d", port);
    status = getaddrinfo(host, portStr, &hints, serverinfo);

    if(status != STATUS_SUCCESS){
        fprintf(stderr, "Error! (getaddrinfo): %s\n", gai_strerror(status));
        // Тут должен быть выход!
        return STATUS_FAILURE;
    }

    return STATUS_SUCCESS;
}

int prepareGetRequest(char buffer, size_t bufferSize, char* hostname, char* path){

}

int debugFunction(int socketdescr){
    char *message = "GET / HTTP/1.0\r\n\r\n";
    char serverReply[2000];
    if (send(socketdescr, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    puts("Data Send\n");

    if(recv(socketdescr, serverReply, 2000 , 0) < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
    puts(serverReply);
}

int openSocket(char *host, int port) {
    int status;
    int socketdescr;
    struct addrinfo * serverinfo;

    status = prepareServerAddrInfo(&serverinfo, port, host);
    if(status != STATUS_SUCCESS){
        // Тут напечатаем ошибку
        return STATUS_FAILURE;
    }

//    status = prepareServerSocketAddr(&server, &serverinfo, port);

    socketdescr = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if(socketdescr == STATUS_FAILURE){
        // Тут напечатаем ошибку
        return STATUS_FAILURE;
    }

    status = connect(socketdescr, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if(status != STATUS_SUCCESS){
        // Тут напечатаем ошибку
        return STATUS_FAILURE;
    }

    freeaddrinfo(serverinfo);

    return socketdescr;
}

int parseURL() {

}

