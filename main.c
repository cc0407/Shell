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
    char** args;
    char* inFile;
    char* outFile;
    int numArgs;
    char* token;
    int background; 
    int out;
    int in;
    int successfulRead;

    //for killing zombies
    pid_t asyncPID;
    int status;

    //for finding > and <
    char* foundIndex;

    while(1) {
        successfulRead = 1; // If there are syntax errors while parsing, this turns to 0
        args = NULL;
        numArgs = 0;
        token = NULL;
        outFile = NULL;
        inFile = NULL;
        background = 0;
        out = 0;
        in = 0;

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
            exitShell();
        }

        // Grab optional args from inBuffer
        token = strtok_r(inBuffer, " ", &inBuffer);
        while (token != NULL) {
            printf("{%s}\n", token);
            printf("a\n");
            
            // Check if '>' + filename were given
            if( (foundIndex = strchr(token, '>')) != NULL) {
                // '>' was provided but no space (i.e. ">test.txt") and its not at the end (i.e. "ls> test.txt")
                if(strlen(token) > 1) {
                    if (*foundIndex != token[0]) { // '>' is not at beginning (i.e. "ls>test.txt")
                        *foundIndex = '\0';
                    }
                    outFile = (char*)(malloc( (strlen(foundIndex+1) + 1) * sizeof(char) ));
                    strcpy(outFile, foundIndex+1);
                    out = 1;
                    if (*foundIndex == token[0]) { // '>' is at beginning (i.e. "ls>test.txt")
                        token = strtok_r(inBuffer, " ", &inBuffer);
                        continue;
                    }
                }
                else{
                    // '>' was provided isolated (i.e. "ls > test"), checks if filename was provided after (i.e. "> test.txt")
                    token = strtok_r(inBuffer, " ", &inBuffer);
                    if (token == NULL) { // Filename not provided
                        printf("Redirect Error, no filename provided.\n");
                        successfulRead = 0;
                        break;
                    }
                    else{ // Filename was provided
                        outFile = (char*)(malloc( (strlen(token) + 1) * sizeof(char) ));
                        strcpy(outFile, token);
                        token = strtok_r(inBuffer, " ", &inBuffer);

                        out = 1;
                        continue;
                    }
                }
            }
            printf("g\n");
            // Check if '<' + filename were given
            if( (foundIndex = strchr(token, '<')) != NULL) {
                // This '<' is not the first '<' to be parsed
                if( in ) {
                    perror("Syntax error: multiple input redirections\n");
                    successfulRead = 0;
                    break;
                }

                // '<' was provided but no space (i.e. "<test.txt") and its not at the end (i.e. "ls< test.txt")
                if(strlen(token) > 1) {
                    // '<' is not at beginning (i.e. "ls<test.txt")
                    if (*foundIndex != token[0]) { 
                        *foundIndex = '\0';
                    }

                    inFile = (char*)(malloc( (strlen(foundIndex+1) + 1) * sizeof(char) ));
                    strcpy(inFile, foundIndex+1);
                    in = 1;

                    // '<' is at beginning (i.e. "ls<test.txt")
                    if (*foundIndex == token[0]) { 

                        //check if there are more redirects in this statement
                        if(strchr(foundIndex + 1, '<') != NULL || strchr(foundIndex + 1, '>') != NULL) {
                            token++;
                            continue;
                        }
                        else {
                            token = strtok_r(inBuffer, " ", &inBuffer);
                            continue;
                        }
                        
                    }
                }
                // '<' was provided isolated (i.e. "ls < test"), checks if filename was provided after (i.e. "< test.txt")
                else{
                    token = strtok_r(inBuffer, " ", &inBuffer);

                    // Filename not provided
                    if (token == NULL) { 
                        printf("Redirect Error, no filename provided.\n");
                        successfulRead = 0;
                        break;
                    }
                    // Filename was provided
                    else{ 
                        inFile = (char*)(malloc( (strlen(token) + 1) * sizeof(char) ));
                        strcpy(inFile, token);
                        token = strtok_r(inBuffer, " ", &inBuffer);
                        in = 1;
                        continue;
                    }
                }
            }
            printf("h\n");
            // arg list is empty, must malloc first
            if( args == NULL ) {
                args = (char**)calloc(1, sizeof(char*));
            }
            // realloc space for new argument
            else {
                args = (char**)realloc(args, (numArgs + 1) * sizeof(char*));
            }

            // allocate new arg string
            args[numArgs] = (char*)(malloc( (strlen(token) + 1) * sizeof(char) ));
            strcpy(args[numArgs], token);

            // parse next token
            token = strtok_r(inBuffer, " ", &inBuffer);
            numArgs++;
        }

        if( strchr(args[numArgs - 1], '&') != NULL) {
            background = 1;
            free(args[numArgs - 1]);
            args = (char**)realloc(args, --numArgs * sizeof(char*));
        }
        if( successfulRead ) {
            // Add null terminated pointer to end of arg list
            args = (char**)realloc(args, ++numArgs * sizeof(char*));
            args[numArgs - 1] = NULL;
            

            //TODO DEBUGGING
            for(int i = 0; i < numArgs; i++) {
                printf("[%s]", args[i]);
            }
            printf("\n");

            newProcess(args[0], args, background, out, in, outFile, inFile);
        }
        if(outFile != NULL) {
            free(outFile);
        }
        if(inFile != NULL) {
            free(inFile);
        }
        free( inputCopy );
        freeArgs(args, numArgs);
    }
}

int newProcess(char* command, char ** args, int bg, int out, int in, char* outFile, char* inFile) {
    pid_t pid;
    int status;
    int IOFlag = 0; // 0 for nothing, 1 for output, 2 for input. Different than params because this is only changed if filename is not empty
    FILE* file;
    int fd;

    pid = fork();
    //printf("PID: %d, %s\n", pid, command);
    addToList( pid );

    if (pid < 0) {
        perror("Fork Failed\n");
        return 1;
    }


    // Handling ">" and "<"
    if( out && pid == 0) {
        if((outFile != NULL)) {
            // file = freopen(outFile, "w+", stdout);
            // if(file == NULL) {
            //     perror("output file: no such file or directory\n");
            // }
            fd = open(outFile, O_WRONLY | O_CREAT, 0777);
            if( fd < 0 ) {
                perror("output file: no such file or directory\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            IOFlag = 1;
        }
        else{
            exit(1);
        }
    }   
    
    if( in && pid == 0) {
        if((inFile != NULL)) {
            fd = open(inFile, O_RDONLY, 0);
            if(fd < 0) {
                perror("input file: no such file or directory\n");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            IOFlag = 2;
        }
        else{
            exit(1);
        }
    }

    if (pid == 0) {
        //printf("Executing %s\n", command);
        if (execvp(command, args) == -1) {
            perror("Command Failed\n");
        }
        // if( IOFlag == 1 ) {
        //     fclose(file);
        // }
        exit(1);
    }
    else if (!bg) {
        waitpid(pid, &status, 0);
        removeFromList( pid );
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

void exitShell() {
    pidNode* currNode;

    // Iteratively kills all active processes
    while( pidList != NULL ) {
        printf("pid: %d\n", pidList->pid);
        kill(pidList->pid, SIGKILL);
        currNode = pidList;
        pidList = pidList->next;
        freeNode(currNode);
    }
    

    printf("myShell terminating...\n");

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
