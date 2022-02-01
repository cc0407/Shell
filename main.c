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
    int successfulParse;

    //for killing zombies
    pid_t asyncPID;
    int status;

    //for finding > and <
    char* foundIndex;

    while(1) {
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
        
        // Parsing input redirection
        successfulParse = parseIORedir(&in, inputCopy, &inFile, '<');
        if( successfulParse == -1 ) {
            perror("Syntax error on input redirection\n");
            free( inputCopy );
            continue;
        }

        // Parsing output redirection
        successfulParse = parseIORedir(&out, inputCopy, &outFile, '>');
        if( successfulParse == -1 ) {
            perror("Syntax error on output redirection\n");
            free( inputCopy );
            continue;
        }

        // Grab optional args from inBuffer
        token = strtok_r(inBuffer, " ", &inBuffer);
        while (token != NULL) {
            printf("{%s}\n", token);

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

        // Add null terminated pointer to end of arg list
        args = (char**)realloc(args, ++numArgs * sizeof(char*));
        args[numArgs - 1] = NULL;
        

        //TODO DEBUGGING
        for(int i = 0; i < numArgs; i++) {
            printf("[%s]", args[i]);
        }
        printf("\n");

        printf("I/0/IF/OF?: %d/%d", in, out);
        if(inFile!=NULL){printf("/%s", inFile);};
        if(outFile!=NULL){printf("/%s", outFile);};
        printf("\n");
        newProcess(args[0], args, background, out, in, outFile, inFile);

        if(inFile != NULL) {
            free(inFile);
        }
        if(outFile != NULL) {
            free(outFile);
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


int parseIORedir(int* flag, char* input, char** filename, char key) {
    char* foundIndex;

    if( (foundIndex = strchr(input, key)) != NULL) {
        // more than one key provided
        if( strchr(foundIndex+1, key) != NULL ) {
            printf("no2\n");
            return -1;
        }
        
        // key is final character, no filename provided
        if(*foundIndex == input[strlen(input) - 1]) {
            printf("no1\n");
            return -1;
        }

        *foundIndex = ' ';
        *filename = findFilename(foundIndex+1);

        if(strlen(*filename) == 0) {
            printf("no3\n");
            return -1;
        }

        clearString(foundIndex, strlen(*filename));
        *flag = 1;
        return 1;
    }

    printf("no4\n");
    *flag = 0;
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
    char blocklist[13] = {'\0', ' ', '>', '<', ',', '&', '/', '\\', ':', '*', '?', '"', '|'};
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
