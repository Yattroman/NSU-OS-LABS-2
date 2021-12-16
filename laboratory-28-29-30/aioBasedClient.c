#include "clientPart.h"

int aioClient(char * host, int port){
    struct addrinfo * serverinfo;
    int socket = openSocket(host, port, &serverinfo);
    if (socket == STATUS_FAILURE) {
        fprintf(stderr, "ебизи");
    }

    freeaddrinfo(serverinfo);
}

