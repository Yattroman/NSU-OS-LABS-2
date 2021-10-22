#include <malloc.h>
#include <string.h>

#define STATUS_FAIL -1
#define STATUS_SUCCESS 0

typedef struct Node{
    char * stringValue;
    struct Node * next;
} Node;

typedef struct List{
    Node * head;
    int length;
} List;

Node* createNode(char* string){
    Node* node = (Node*) malloc(sizeof(Node));

    if(node == NULL){
        fprintf(stderr, "createNode. There is a trouble with allocating memory with malloc for new list node.\n");
        return NULL;
    }

    int strLength = strlen(string);
    char * newStr = (char*) malloc(strLength + 1);

    if(newStr == NULL){
        fprintf(stderr, "createNode. There is a trouble with allocating memory with malloc for new node's string.\n");
        free(node);
        return NULL;
    }

    memccpy(newStr, string, strLength, sizeof(char)*strLength );

    node->next = NULL;
    node->stringValue = newStr;
    newStr[strLength] = '\0';

    return node;
}

int push(List* list, Node* node){

    if(node == NULL){
        fprintf(stderr, "Invalid node\n");
        free(list);
        return STATUS_FAIL;
    }

    if(list == NULL){
        fprintf(stderr, "Invalid list\n");
        free(node);
        return STATUS_FAIL;
    }

    if(list->head == NULL){
        list->head = node;
        list->length += 1;
        return STATUS_SUCCESS;
    }

    node->next = list->head;
    list->head = node;
    list->length += 1;

    return STATUS_SUCCESS;
}

void initList(List* list){
    list->head = NULL;
    list->length = 0;
}

void printList(List* list){
    if(list->head == NULL){
        printf("%s", "List is empty!");
        return;
    }

    Node * current = list->head;

    while(current != NULL) {
        printf("%s\n", current->stringValue);
        current = current->next;
    }

    printf("List length %d.", list->length);
}

void freeNode(Node* node){
    free(node->stringValue);
    free(node);
    node = NULL;
}

void freeList(List* list){
    Node *current = list->head;
    while (current) {
        Node *temp = current;
        current = current->next;
        freeNode(temp);
    }
    list = NULL;
}