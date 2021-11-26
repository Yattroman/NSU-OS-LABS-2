#ifndef NSU_OS_LABS_2_LIST_H
#define NSU_OS_LABS_2_LIST_H

#include <malloc.h>

#include <string.h>
#include <pthread.h>
#include "util.h"

typedef struct Node {
    char *stringValue;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    int length;
} List;

pthread_mutex_t listLock;

Node *createNode(char *string);

void freeNode(Node *node);

int push(List *list, Node *node);

void printList(List *list);

void initList(List *list);

void freeList(List *list);

void sortList(List *list);

#endif //NSU_OS_LABS_2_LIST_H
