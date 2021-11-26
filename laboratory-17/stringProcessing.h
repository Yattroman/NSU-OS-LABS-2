#ifndef NSU_OS_LABS_2_STRINGPROCESSING_H
#define NSU_OS_LABS_2_STRINGPROCESSING_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

int expandInputBuffer(char **inputHolder, size_t *bufferSize);

int readLine(char **line, size_t *currentLineLength);

void truncateNewLineCharacter(char *line, size_t *lineLength);

int readString(char **string);

#endif //NSU_OS_LABS_2_STRINGPROCESSING_H
