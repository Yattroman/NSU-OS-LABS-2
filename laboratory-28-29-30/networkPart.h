#ifndef NSU_OS_LABS_2_NETWORKPART_H
#define NSU_OS_LABS_2_NETWORKPART_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int parseURL();
int openSocket(char *host, int port, struct addrinfo ** serverinfo);
char *prepareGetRequest(char *buffer, size_t bufferSize, char *url);

#endif //NSU_OS_LABS_2_NETWORKPART_H
