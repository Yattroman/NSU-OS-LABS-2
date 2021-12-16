#ifndef NSU_OS_LABS_2_CLIENTPART_H
#define NSU_OS_LABS_2_CLIENTPART_H

#include "networkPart.h"
#include "defAssigns.h"

#define MAX_LINES 16

int selectClient(char* url, int port);
int aioClient(char* url, int port);
int threadsClient(char* url, int port);

#endif
