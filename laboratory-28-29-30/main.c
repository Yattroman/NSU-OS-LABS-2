#include "clientPart.h"
#define MIN_REQ_ARGS 1

int main(int argc, char** argv) {

    if (argc < MIN_REQ_ARGS) {
        fprintf(stderr, "usage: <progexecfile> <url>\n");
        return STATUS_FAILURE;
    }

//    selectClient("ccfit.nsu.ru", 80);
    threadsClient("ccfit.nsu.ru", 80);
//    aioClient("www.google.com", 80);
}

