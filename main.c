#include "main.h"

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
    char **args[2];
    char *inFile[2];
    char *outFile[2];
    int numArgs[2];
    int background; 
    int fd[2]; // This is just to pass down into newProcess() for recursive calls, not used elsewhere
    int pipe;
    int successfulParse;
    char* pipeIndex;

    //for killing zombies
    pid_t asyncPID;
    int status;

    while(1) {

        // Initialize all variables
        for(int i = 0; i < 2; i++) {
            args[i] = NULL;
            numArgs[i] = 0;
            outFile[i] = NULL;
            inFile[i] = NULL;
        }
        background = 0;
        pipe = 0;

        // wait for user input
        printf("> ");
        inBuffer = readInputLine();
        inputCopy = inBuffer;

        // Kill any completed background processes
        asyncPID = waitpid(-1, &status, WNOHANG);
        if( asyncPID > 0) {
            removeFromList( asyncPID );
            printf("Done            : %d\n", asyncPID);
        }

        // Internal shell commands
        if( strcmp(inBuffer, "exit") == 0 ) {
            free( inputCopy );
            exitShell(getpgrp());
        }
        
        
        if( (pipeIndex = strchr(inputCopy, '|')) != NULL) {
            // Split the input line in two and call parse twice
            *pipeIndex = '\0';
            printf("[%s] [%s]\n", inputCopy, pipeIndex + 1);
            pipe = 1;
            if( parseInput(inputCopy, &args[0], &numArgs[0], &outFile[0], &inFile[0], &background) == 0) {
                free( inputCopy );
                continue;
            }
            if(args[1] == 0)
            if( parseInput(pipeIndex + 1, &args[1], &numArgs[1], &outFile[1], &inFile[1], &background) == 0 ) {
                free( inputCopy );
                continue;
            }
        }
        else {
            // Parse the line
            if( parseInput(inputCopy, &args[0], &numArgs[0], &outFile[0], &inFile[0], &background) == 0) {
                free( inputCopy );
                continue;
            }
        }


        

        //TODO DEBUGGING
        /*for(int i = 0; i < 2; i++) {
            for(int j = 0; j < numArgs[i]; j++) {
                if(args[i] == NULL) {
                    break;
                }
                printf("[%s]", args[i][j]);
            }   
            printf("\n");
            printf("IF/OF?:");
            if(inFile[i]!=NULL){printf(" [%s]", inFile[i]);};
            if(outFile[i]!=NULL){printf(" [%s]", outFile[i]);};
            printf("\n");
        }*/
        

        newProcess(args, background, pipe, fd, outFile, inFile);

        // Free all variables used
        for( int i = 0; i < 2; i++){
            if(inFile[i] != NULL) {
                free(inFile[i]);
            }
            if(outFile[i] != NULL) {
                free(outFile[i]);
            }
            freeArgs(args[i], numArgs[i]);
        }
        free( inputCopy ); 
    }
        
}

int parseInput(char* inputLine, char*** args, int* numArgs, char** outFile, char** inFile, int* background) {
    char** argList = *args;
    char* token = NULL;
    // Parsing input redirection
    if( parseIORedir(inputLine, inFile, '<') == -1 ) {
        perror("Syntax error on input redirection\n");
        return 0;
    }

    if( parseIORedir(inputLine, outFile, '>') == -1 ) {
        perror("Syntax error on output redirection\n");
        return 0;
    }

    // Grab optional args from inBuffer
    token = strtok_r(inputLine, " ", &inputLine);
    while (token != NULL) {


        // arg list is empty, must malloc first
        if( argList == NULL ) {
            argList = (char**)calloc(1, sizeof(char*));
        }
        // realloc space for new argument
        else {
            argList = (char**)realloc(argList, (*numArgs + 1) * sizeof(char*));
        }

        // allocate new arg string
        argList[*numArgs] = (char*)(malloc( (strlen(token) + 1) * sizeof(char) ));
        strcpy(argList[*numArgs], token);

        // parse next token
        token = strtok_r(inputLine, " ", &inputLine);
        *numArgs = *numArgs + 1;
    }
    if(argList == NULL) {
        return 0;
    }

    if(strchr(argList[*numArgs - 1], '&') != NULL) {
        *background = 1;
        free(argList[*numArgs - 1]);
        *numArgs = *numArgs - 1;
        argList = (char**)realloc(argList, *numArgs * sizeof(char*));
    }

    // Add null terminated pointer to end of arg list
    *numArgs = *numArgs + 1;
    argList = (char**)realloc(argList, *numArgs * sizeof(char*));
    argList[*numArgs - 1] = NULL;

    *args = argList;
}

int pipedProcessHandler(char **args[2], int bg, int p, int fd[2], char *outFile[2], char *inFile[2]) {

}

// args must have either 1 or 2 arrays of strings
// bg flags if the command should be run in the background
// pipe flags if the command should be piped
// fd[2] are the file descriptors to be used if pipe is set to 1 or -1
// outFile and inFile are filenames used to redirect IO to/from a file
void newProcess(char **args[2], int bg, int p, int fd[2], char *outFile[2], char *inFile[2]) {

    int j=0;
    printf("process created\n");
    int saved_stdin;
    int saved_stdout;

    pid_t pid;
    int status;
    int IOfd;

    // Pipe if flag was set
    if( p == 1 ) {
        saved_stdin = dup(0);
        saved_stdout = dup(1);
        printf("made pipe\n");
        if( pipe(fd) == -1 ) {
            perror("pipe");
            exit(1);
        }
    }
    // Fork and add PID to linked list for deletion on exit
    pid = fork();
    if (pid < 0) {
        perror("Fork Failed\n");
        return;
    }    

    if (pid == 0) {
        if( p == 1 ) {
            close(1);
            close(fd[0]);
            dup2 (fd[1], 1);
        }
        if( p == -1 ) {
            close(0);
            close(fd[1]);
            dup2 (fd[0], 0);
        }
        // "<" handler
        if((inFile[0] != NULL)) {
            IOfd = open(inFile[0], O_RDONLY, 0);
            if(IOfd < 0) {
                perror(inFile[0]);
                exit(1);
            }
            dup2(IOfd, 0);
            close(IOfd);
        }

        // ">" handler
        if((outFile[0] != NULL) && pid == 0) {
            IOfd = open(outFile[0], O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if( IOfd < 0 ) {
                perror(outFile[0]);
                
                exit(1);
            }
            dup2(IOfd, 1);
            close(IOfd);
        }

        //printf("Executing %s\n", command);
        if (execvp(args[0][0], args[0]) == -1) {
            perror(args[0][0]);
        }
    }
    else {
        if( p == 1 ) {
            // Move second arguments over to index 0 so recursive call can use them
            char **newArgs[2] = {args[1], '\0'};
            char *newOutFile[2] = {outFile[1], '\0'};
            char *newInFile[2] = {inFile[1], '\0'};
            newProcess(newArgs, 0, -1, fd, newOutFile, newInFile); 

            close(fd[0]);
            close(fd[1]);
            dup2(saved_stdin, 0);
            dup2(saved_stdout, 1);
        }
        else if(p == 0 && !bg) {
                waitpid(pid, &status, 0);
        }
        printf("Child Complete\n");
        
    }

}

char* readInputLine() { //TODO TRIM INPUT
    int bufferLen = 100;
    char* buffer;
    int charAmt = 0;
    char c;

    // Allocate initial buffer
    buffer = calloc(bufferLen, bufferLen * sizeof(char));
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

void exitShell(pid_t subprocessGroupID) {
    pidNode* currNode;

    // Iteratively kills all active processes
    /*while( pidList != NULL ) {
        printf("pid: %d\n", pidList->pid);
        kill(pidList->pid, SIGKILL);
        currNode = pidList;
        pidList = pidList->next;
        freeNode(currNode);
    }*/
    
    kill(subprocessGroupID, SIGKILL);
    printf("myShell terminating...\n");

    printf("[Process completed]\n");
    exit(EXIT_SUCCESS);
}


int parseIORedir(char* input, char** filename, char key) {
    char* foundIndex;

    if( (foundIndex = strchr(input, key)) != NULL) {
        // more than one key provided
        if( strchr(foundIndex+1, key) != NULL ) {
            return -1;
        }
        
        // key is final character, no filename provided
        if(*foundIndex == input[strlen(input) - 1]) {
            return -1;
        }

        *foundIndex = ' ';
        *filename = findFilename(foundIndex+1);

        if(strlen(*filename) == 0) {
            return -1;
        }

        clearString(foundIndex, strlen(*filename));
        return 1;
    }

    return 0;
}

void clearString(char* string, int amt){

    if(amt > strlen(string)) {
        amt = strlen(string);
    }

    // trimming input
    while(*(string) == ' ') {
        *string++;
    }

    for(int i = 0; i < amt; i++) {
        string[i] = ' ';
    }

}

char* findFilename(char* string) {
    char* filename;
    int count;
    char blocklist[13] = {'\0', ' ', '>', '<', '|'};
    filename = (char*)(malloc(1 * sizeof(char) ));
    *filename = '\0';
    count = 1;

    // trimming input
    while(*(string) == ' ') {
        *string++;
    }

    for(int i = 0; i < strlen(string); i++) {
        for(int j = 0; j < 13; j++) {
            if(string[i] == blocklist[j]) {
                return filename;
            }
        }
        filename = (char*)realloc(filename, ++count);
        strncat(filename, string + i, 1);
    }
    return filename;
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

void clearInputBuffer() {
    char c;
    do {
        c = getchar();
    } while( c != '\n' && c != EOF);
}
