#include "clientPart.h"

int aioClient(char * host, int port){
    int socket = openSocket(host, port);
    if (socket == STATUS_FAILURE) {
        fprintf(stderr, "ебизи");
    }
}

