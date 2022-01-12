#include "main.h"

pidNode* pidList;
int pidAmt;

int main(int argc, char* argv[])
{
    printf("Welcome!\n");
    inputLoop();

    return 0;
}

void inputLoop() {
    //TODO i dont like that this has a max value
    char inBuffer[MAX_STR];

    while(1) {
        printf("> ");
        readInputLine( inBuffer );

        if( strcmp(inBuffer, "exit") == 0 ) {
            exitShell();
        }

        //TODO handle the input and call functions
        newSynchronousProcess();

    }
}

void exitShell() {
    printf("myShell terminating...\n");
    kill(0, SIGKILL); // TODO change 0 to loop through the PID of all children that way it doesnt remove the parent as well

    printf("[Process completed]");
    exit(EXIT_SUCCESS);
}

void readInputLine( char* buffer ) {
    int bufferLen;
    char *result = fgets(buffer, MAX_STR, stdin);

    // Input Validation
    if(result == NULL) {
        buffer = "";
        return;
    }

    bufferLen = strlen(buffer);
    if(bufferLen == 0) {
        buffer = "";
        return;
    }

    // Removing newline if present
    if(buffer[bufferLen - 1] == '\n') {
        buffer[bufferLen - 1] = '\0';
    }

}

int newSynchronousProcess() {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }
    else if (pid == 0) {
        execlp("/bin/ls", "ls", NULL);
    }
    else {
        wait(&status);
        printf("Child Complete\n");
    }
    return 0;
}

void testLinkedList() {
    addToList(1);
    printList();
    addToList(2);
    addToList(3);
    addToList(4);
    printList();
    removeFromList(1);
    printList();
    removeFromList(3);
    printList();
    freeList();
    printList();
}

void addToList( int pid ) {
    pidNode* tempNode;
    pidNode* newNode = (pidNode*)malloc(sizeof(pidNode));
    newNode->pid = pid;
    newNode->next = NULL;

    // List is empty
    if( pidList == NULL ) {
        pidList = newNode;
    }

    // Add node to end of list
    else{
        tempNode = pidList;
        for(int i = 0; i < pidAmt - 1; i++) {
            tempNode = tempNode->next;
        }

        tempNode->next = newNode;
    }

    pidAmt++;
    return;
}

void printList(){
    printf("----\n");
    pidNode* tempNode = pidList;
    for(int i = 0; i < pidAmt; i++) {
        printf("PID: %d\n", tempNode->pid);
        tempNode = tempNode->next;
    }
}

void removeFromList( int pid ) {
    pidNode* prevNode;
    pidNode* currNode;

    // Pid list is empty
    if( pidList == NULL ) {
        return;
    }

    // Pid is head of list
    if( pidList->pid == pid ) {
        currNode = pidList;
        pidList = pidList->next;

        freeNode(currNode);
    }
    // Pid might be inside list
    else {
        currNode = pidList;
        
        // iterate through each node
        while(currNode->next != NULL) {
            prevNode = currNode;
            currNode = currNode->next;

            // Pid found
            if( currNode->pid == pid ) {
                prevNode->next = currNode->next;
                freeNode(currNode);
            }
        }
    }
}

void freeList() {
    pidNode* currNode;
    
    // Iteratively remove nodes from list
    while( pidList != NULL ) {
        currNode = pidList;
        pidList = pidList->next;
        freeNode(currNode);
    }

}

void freeNode(pidNode* node) {
    free(node);
    pidAmt--;
}

