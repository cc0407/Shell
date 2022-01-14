#include "main.h"

//TODO THIS DOESNT REALLY WORK THE WAY I THINK IT DOES, GLOBAL VARIABLES ARE NOT SHARED BETWEEN TWO PROCESSES
pidNode* pidList;
int pidAmt;

int main(int argc, char* argv[]) {
    printf("Welcome!\n");
    inputLoop();

    return 0;
}

void inputLoop() {
    char* inBuffer = NULL;
    char* inputCopy; // For freeing
    //char* command;
    char** args;
    int numArgs;
    char* token;
    int background;

    //for killing zombies
    pid_t asyncPID;
    int status;

    while(1) {
        args = NULL;
        numArgs = 0;
        token = NULL;
        background = 0;


        printf("> ");
        inBuffer = readInputLine();
        inputCopy = inBuffer;

        // Kill any completed background processes
        asyncPID = waitpid(-1, &status, WNOHANG);
        if( asyncPID > 0) {
            printf("async process completed, killed: %d\n", asyncPID);
        }

        // Internal shell commands
        if( strcmp(inBuffer, "exit") == 0 ) {
            free( inputCopy );
            exitShell();
        }

        // Grab optional args from inBuffer
        token = strtok_r(inBuffer, " ", &inBuffer);
        while (token != NULL) {
            //printf("token: %s\n", token);

            // Check if background flag was given
            if( strcmp(token, "&") == 0 ) {
                background = 1;
                token = strtok_r(inBuffer, " ", &inBuffer);
                continue;
            }

            // arg list is empty, must malloc first
            if( args == NULL ) {
                args = (char**)calloc(0, numArgs * sizeof(char*));
            }
            else {
                args = (char**)realloc(args, numArgs * sizeof(char*));
            }

            // allocate space for new arg
            args[numArgs] = (char*)(malloc( sizeof(char) * strlen(token) ));
            strcpy(args[numArgs], token);

            token = strtok_r(inBuffer, " ", &inBuffer);
            numArgs++;
        }

        // Add null terminated pointer to end of arg list
        args = (char**)realloc(args, ++numArgs * sizeof(char*));
        args[numArgs - 1] = NULL;
        

        //TODO DEBUGGING
        for(int i = 0; i < numArgs; i++) {
            printf("[%s]", args[i]);
        }
        printf("\n");

        newProcess(args[0], args, background);
        free( inputCopy );
    }
}

int newProcess(char* command, char ** args, int bg) {
    pid_t pid;
    int status;
    
    pid = fork();
    printf("PID: %d, %s\n", pid, command);

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }
    else if (pid == 0) {
        //printf("Executing %s\n", command);
        if (execvp(command, args) == -1) {
            fprintf(stderr, "Command Failed\n");
        }
        exit(1);
    }
    else if (!bg) {
        waitpid(pid, &status, 0);
        printf("Child Complete\n");
    }

    return 0;
}

char* readInputLine() { //TODO TRIM INPUT
    int bufferLen = 100;
    char* buffer;
    int charAmt = 0;
    char c;

    // Allocate initial buffer
    buffer = malloc(bufferLen * sizeof(char));
    c = getchar();

    while( c != '\n' && c != EOF) {
        if( charAmt >= bufferLen ) {
            buffer = realloc( buffer, (bufferLen += 100) * sizeof(char));
        }
        buffer[charAmt] = c;
        charAmt++;
        c = getchar();
    }

    return buffer;
}

void exitShell() {
    pidNode* currNode;
    printf("myShell terminating...\n");

    // Iteratively kills all active processes
    while( pidList != NULL ) {
        //printf("pid: %d\n", pidList->pid);
        kill(pidList->pid, SIGKILL);
        currNode = pidList;
        pidList = pidList->next;
        freeNode(currNode);
    }

    printf("[Process completed]\n");
    exit(EXIT_SUCCESS);
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

void freeArgs( char* args[], int numArgs ) {
    for( int i = 0; i < numArgs; i++ ) {
        free(args[i]);
    }
    free(args);
}
