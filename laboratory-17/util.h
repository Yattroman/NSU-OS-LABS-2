#ifndef NSU_OS_LABS_2_UTIL_H
#define NSU_OS_LABS_2_UTIL_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

#define BUFFER_DEF_LENGTH 256

#define ENDLINE_CHARACTER '\n'
#define TRUE 1

#define STATUS_SUCCESS 0
#define STATUS_FAILURE -1
#define STATUS_FAILURE_MEMORY NULL
#define STATUS_MALLOC_FAIL NULL

char errorBuffer[BUFFER_DEF_LENGTH];

void verifyFunctionsByErrno(int returnCode, const char *functionName);

void verifyPthreadFunctions(int returnCode, const char *functionName);

#endif
