#include "list.h"

#define STATUS_FAILURE -1
#define STATUS_SUCCESS 0
#define STRINGS_ORDERED(STRING1, STRING2) (strcmp(STRING1, STRING2) <= 0)

Node *createNode(char *string) {
    Node *node = (Node *) malloc(sizeof(Node));

    if (node == NULL) {
        fprintf(stderr, "createNode. There is a trouble with allocating memory with malloc for new list node.\n");
        return NULL;
    }

    int strLength = strlen(string);
    char *newStr = (char *) malloc(strLength + 1);

    if (newStr == NULL) {
        fprintf(stderr, "createNode. There is a trouble with allocating memory with malloc for new node's string.\n");
        free(node);
        return NULL;
    }

    memccpy(newStr, string, strLength, sizeof(char) * strLength);

    node->next = NULL;
    node->stringValue = newStr;
    newStr[strLength] = '\0';

    return node;
}

void freeNode(Node *node) {
    free(node->stringValue);
    free(node);
    node = NULL;
}

void freeList(List *list) {
    verifyPthreadFunctions(pthread_mutex_lock(&listLock), "pthread_mutex_lock");
    Node *current = list->head;
    while (current) {
        Node *temp = current;
        current = current->next;
        freeNode(temp);
    }
    list = NULL;
    verifyPthreadFunctions(pthread_mutex_unlock(&listLock), "pthread_mutex_unlock");

    verifyPthreadFunctions(pthread_mutex_destroy(&listLock), "pthread_mutex_destroy");
}

int push(List *list, Node *node) {
    if (node == NULL) {
        fprintf(stderr, "Invalid node\n");
        return STATUS_FAILURE;
    }

    if (list == NULL) {
        fprintf(stderr, "Invalid list\n");
        freeNode(node);
        return STATUS_FAILURE;
    }

    verifyPthreadFunctions(pthread_mutex_lock(&listLock), "pthread_mutex_lock");
    node->next = list->head;
    list->head = node;
    list->length += 1;
    verifyPthreadFunctions(pthread_mutex_unlock(&listLock), "pthread_mutex_unlock");

    return STATUS_SUCCESS;
}

void initList(List *list) {
    list->head = NULL;
    list->length = 0;
}

void printList(List *list) {
    if (list->head == NULL) {
        printf("%s", "List is empty!\n");
        return;
    }

    verifyPthreadFunctions(pthread_mutex_lock(&listLock), "pthread_mutex_lock");
    for (Node *iNode = list->head; iNode != NULL; iNode = iNode->next) {
        printf("%s\n", iNode->stringValue);
    }
    printf("List length %d.\n", list->length);
    verifyPthreadFunctions(pthread_mutex_unlock(&listLock), "pthread_mutex_unlock");
}

void swap(Node *a, Node *b) {
    char *box = a->stringValue;
    a->stringValue = b->stringValue;
    b->stringValue = box;
}

void sortList(struct List *list) {
    verifyPthreadFunctions(pthread_mutex_lock(&listLock), "pthread_mutex_lock");

    for (Node *iNode = list->head; iNode != NULL; iNode = iNode->next) {
        for (Node *jNode = iNode->next; jNode != NULL; jNode = jNode->next) {
            if (!STRINGS_ORDERED(iNode->stringValue, jNode->stringValue)) {
                swap(iNode, jNode);
            }
        }
    }

    verifyPthreadFunctions(pthread_mutex_unlock(&listLock), "pthread_mutex_unlock");
}