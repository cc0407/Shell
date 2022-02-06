#include "main.h"

pidNode* pidList;
int pidAmt;
env envList[ENVAMT]; // 0 is myPATH, 1 is myHISTFILE, 2 is myHOME
FILE * histFile;

int main(int argc, char* argv[]) {
    initENV();
    loadProfile();
    //histFile = fopen()
    printf("Welcome!\n");
    inputLoop();

    return 0;
}

void initENV() {
    // Allocate space for myHOME in envList[2]
    strcpy(envList[2].name, "myHOME");
    envList[2].value = (char*)malloc(1024 * sizeof(char)); // mallloc with large buffer
    strcpy(envList[2].value, getenv("HOME"));
    envList[2].value = realloc(envList[2].value, strlen(envList[2].value) + 1); // shorten the buffer to fit string

    // Allocate space for myHISTFILE in envList[1]
    strcpy(envList[1].name, "myHISTFILE");
    envList[1].value = (char*)malloc( (strlen(envList[2].value) + 18) * sizeof(char)); // allocate space for path variable and 
    strcpy(envList[1].value, getenv("HOME"));
    strcat(envList[1].value, "/.CIS3110_History");

    // Allocate space for myPATH in envList[0]
    strcpy(envList[0].name, "myPATH");
    envList[0].value = (char*)malloc( 5 * sizeof(char)); // allocate space for path variable and 
    strcpy(envList[0].value, "/bin");
}

void loadProfile() {
    char* inBuffer;
    int readFlag = 1;
    char * EOFIndex;

    // Build path to profile
    char *filePath = (char*)malloc( (strlen(envList[2].value) + 18) * sizeof(char));
    char profileName[18] = "/.CIS3110_profile";
    strcpy(filePath, envList[2].value);
    strcat(filePath, profileName);

    // Attempt to open profile
    FILE* profile = fopen(filePath, "r");
    free(filePath);
    if(profile == NULL) {
        fprintf(stderr, "Could not open profile ~/.CIS3110_profile.\n");
        return;
    }

    // Iterate through each line, running the command. Stops at EOF
    while(readFlag) {
        inBuffer = readInputLine(profile);
        // EOF found
        if((EOFIndex = strchr(inBuffer, EOF)) != NULL) {
            readFlag = 0;
            // remove EOF from final character
            *EOFIndex = '\0'; 
        }
        parseLine(inBuffer);
    }

    fclose(profile);
}

void inputLoop() {
    char* inBuffer = NULL;
    //char **args[2];
    //char *inFile[2];
    //char *outFile[2];
    //int background; 
    //int successfulParse;

    //for killing zombies
    pid_t asyncPID;
    int status;

    while(1) {
        
        // Kill any completed background processes
        asyncPID = waitpid(-1, &status, WNOHANG);
        if( asyncPID > 0) {
            removeFromList( asyncPID );
            printf("Done            : %d\n", asyncPID);
        }

        // wait for user input
        printf("> ");
        inBuffer = readInputLine(NULL);
        parseLine(inBuffer);
        
    }
        
}
int parseLine (char* inputStr) {
        char* pipeIndex;
        char* bgIndex;
        int pipe;
        char **args[2];
        char *inFile[2];
        char *outFile[2];
        int background; 
        //int successfulParse;


        // Replace all shell variables with their respective value
        replaceVarInLine(&inputStr);

        // Save a copy pointer to inputStr for freeing
        char* inputCopy = inputStr;

        // Initialize all variables
        for(int i = 0; i < 2; i++) {
            args[i] = NULL;
            outFile[i] = NULL;
            inFile[i] = NULL;
        }
        background = 0;
        pipe = 0;

        

        // & and | parsing
        if((bgIndex = strchr(inputCopy, '&')) != NULL) {
            background = 1;
            clearString(bgIndex, 1);
        }

        // If piping, split the input line in two and call parse twice
        if( (pipeIndex = strchr(inputCopy, '|')) != NULL) {
            *pipeIndex = '\0';
            //printf("[%s] [%s]\n", inputCopy, pipeIndex + 1);
            pipe = 1;
            if( parseCommand(inputCopy, &args[0], &outFile[0], &inFile[0], &background) == 0) {
                free( inputStr );
                return 0;
            }
            if( parseCommand(pipeIndex + 1, &args[1], &outFile[1], &inFile[1], &background) == 0 ) {
                free( inputStr );
                return 0;
            }
        }
        // Not piping, parse the whole line
        else {
            if( parseCommand(inputCopy, &args[0], &outFile[0], &inFile[0], &background) == 0) {
                free( inputStr );
                return 0;
            }
        }
        
        free( inputStr ); 

        /* Internal Shell Commands */
        if( strcmp(args[0][0], "exit") == 0 ) {
            freeLineVariables(args, outFile, inFile);
            exitShell();
        }
        else if( strcmp(args[0][0], "export") == 0 ) {
            exportENV( args[0] );
            freeLineVariables(args, outFile, inFile);
            return 1;
        }
        /* End Internal Shell Commands */

        if(pipe) {
            pipedProcessHandler(args, background, outFile, inFile);
        }
        else {
            processHandler(args[0], background, outFile[0], inFile[0]);
        }

        // Free all variables used
        freeLineVariables(args, outFile, inFile);
}

int parseCommand(char* commandStr, char*** args, char** outFile, char** inFile, int* background) {
    char** argList = *args;
    char* token = NULL;
    int numArgs = 0;

    // Parsing input redirection
    if( parseIORedir(commandStr, inFile, '<') == -1 ) {
        perror("Syntax error on input redirection\n");
        return 0;
    }

    if( parseIORedir(commandStr, outFile, '>') == -1 ) {
        perror("Syntax error on output redirection\n");
        return 0;
    }

    // Grab optional args from inBuffer
    token = strtok_r(commandStr, " ", &commandStr);
    while (token != NULL) {


        // arg list is empty, must malloc first
        if( argList == NULL ) {
            argList = (char**)calloc(1, sizeof(char*));
        }
        // realloc space for new argument
        else {
            argList = (char**)realloc(argList, (numArgs + 1) * sizeof(char*));
        }

        // allocate new arg string
        argList[numArgs] = (char*)(malloc( (strlen(token) + 1) * sizeof(char) ));
        strcpy(argList[numArgs], token);

        // parse next token
        token = strtok_r(commandStr, " ", &commandStr);
        numArgs++;
    }
    if(argList == NULL) {
        return 0;
    }

    // Add null terminated pointer to end of arg list
    numArgs++;
    argList = (char**)realloc(argList, numArgs * sizeof(char*));
    argList[numArgs - 1] = NULL;

    *args = argList;
}

void pipedProcessHandler(char **args[2], int bg, char *outFile[2], char *inFile[2]) {
    int status;
    pid_t pid1, pid2;
    int pResult1, pResult2; // 1 if command ran successfully, 0 otherwise

    // Create pipe with new file descriptors
    int fd[2];
    pipe(fd);

    // Create processes for both ends of pipe, running the commands
    pResult1 = newProcess(args[0], bg, 1, fd, &pid1, outFile[0], inFile[0]);
    pResult2 = newProcess(args[1], bg, -1, fd, &pid2, outFile[1], inFile[1]);

    // Close file descriptors
    close(fd[0]);
    close(fd[1]);

    // Wait for process to finish or add it to async list (if the command ran correctly)
    if(!bg) {
        if(pResult1) {
            waitpid(pid1, NULL, 0);
        }
        if(pResult2) {
            waitpid(pid2, NULL, 0);
        }
    }
    else{
        if(pResult1) {
            addToList(pid1);
        }
        if(pResult2) {
            addToList(pid2);
        }
    }
}


void processHandler(char **args, int bg, char *outFile, char *inFile) {
    int status;
    pid_t pid1;
    int fd[2];
    int pResult; // 1 if command ran successfully, 0 otherwise

    // Create new process, running its command
    pResult = newProcess(args, bg, 0, fd, &pid1, outFile, inFile);

    // Wait for process to finish or add it to async list (if the command ran correctly)
    if(!bg) {
        if(pResult) {
            waitpid(pid1, NULL, 0);
        }
    }
    else{
        if(pResult) {
            addToList(pid1);
        }
    }

}


// args must have either 1 or 2 arrays of strings
// bg flags if the command should be run in the background
// pipe flag either 1 for left side of pipe, -1 for right side of pipe, or 0 for no pipe
// fd[2] are the file descriptors to be used if pipe is set to 1 or -1
// outFile and inFile are filenames used to redirect IO to/from a file
int newProcess(char **args, int bg, int pipeFlag, int fd[2], pid_t *childPid, char *outFile, char *inFile) {

    pid_t pid;
    int IOfd;

    // Fork and add PID to linked list for deletion on exit
    pid = fork();
    if (pid < 0) {
        perror("Fork Failed\n");
        return 0;
    }

    if (pid == 0) {
        // Pipe handling
        if( pipeFlag == 1 ) {
            close(1);
            close(fd[0]);
            dup2 (fd[1], 1);
        }
        if( pipeFlag == -1 ) {
            close(0);
            close(fd[1]);
            dup2 (fd[0], 0);
        }

        // "<" handler
        if((inFile != NULL)) {
            IOfd = open(inFile, O_RDONLY, 0);
            if(IOfd < 0) {
                perror(inFile);
                freeArgs(args);
                exit(1);
            }
            dup2(IOfd, 0);
            close(IOfd);
        }

        // ">" handler
        if((outFile != NULL) && pid == 0) {
            IOfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if( IOfd < 0 ) {
                perror(outFile);
                freeArgs(args);
                exit(1);
            }
            dup2(IOfd, 1);
            close(IOfd);
        }

        // Execute command
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
            freeArgs(args);
            exit(1);
        }
    }
    else {
        *childPid = pid;
        return 1;
    }

}

//if infile is NULL, it will read from stdin
char* readInputLine(FILE* infile) {
    int bufferLen = 100;
    char* buffer;
    int charAmt = 0;
    char c;
    int loopFlag = 1;

    // Allocate initial buffer
    buffer = calloc(bufferLen, bufferLen * sizeof(char));
    
    do {
        if(infile == NULL) {
            c = getchar();
        }
        else {
            c = fgetc(infile);
        }

        // Break statements depending of if reading from stdin or file
        if(infile == NULL) {
            if (c == '\n' || c == EOF) {
                break;
            }
        }
        else {
            if (c == '\n') {
                break;
            }
            // Keep EOF in string so it can be used when reading profile in
            else if( c == EOF ) {
                loopFlag = 0;
            }
        }

        if( charAmt >= bufferLen ) {
            buffer = realloc( buffer, (bufferLen += 100) * sizeof(char));
        }
        buffer[charAmt] = c;
        charAmt++;
    } while(loopFlag);

    return buffer;
}

void exitShell() {
    pidNode* currNode;

    // Free all ENV variables
    for( int i = 0; i < ENVAMT; i++ ) {
        free(envList[i].value);
    }

    // Iteratively kills all active processes
    while( pidList != NULL ) {
        printf("pid: %d\n", pidList->pid);
        kill(pidList->pid, SIGKILL);
        currNode = pidList;
        pidList = pidList->next;
        freeNode(currNode);
    }

    printf("myShell terminating...\n");

    printf("\n[Process completed]\n");
    exit(EXIT_SUCCESS);
}

void replaceVarInLine(char** inputStr) {
    char envStr[12];
    char *foundStr; // The first occurence of the env variable that's being searched for
    char *afterStr; // The rest of inputStr after foundStr
    char *newStr;   // the newly built string with the expanded value inside
    char *tempStr; //temp pointer to original string, for freeing
    int newSize;

    for(int i = 0; i < ENVAMT; i++) {

        // Build the environment string with $ in-front
        strcpy(envStr, "$");
        strcat(envStr, envList[i].name);

        if( (foundStr = strstr(*inputStr, envStr)) != NULL ) {
            // allocate and store all characters after the string we found
            afterStr = (char*)malloc( (strlen( foundStr + strlen(envStr) )  + 1) * sizeof(char) ); 
            strcpy(afterStr, foundStr + strlen(envStr));

            // Calculate total amount of new characters
            *foundStr = '\0';

            newSize = strlen(*inputStr) + strlen(envList[i].value) + strlen(afterStr) + 1;

            // create new string, add before foundStr, envList[i].value, and afterStr
            newStr = (char*)malloc(newSize * sizeof(char));
            strcpy(newStr, *inputStr);
            strcat(newStr, envList[i].value);
            strcat(newStr, afterStr);

            // Free temp strings
            tempStr = *inputStr;
            *inputStr = newStr;
            free(tempStr);
            free(afterStr);
        }
    }
}

void exportENV( char **args ) {
    // Input validation
    if(args == NULL) {
        return;
    }

    char* argCopy;
    char* token = NULL;
    char* newString = NULL;
    int nameFoundFlag = 0;

    // Print all variables if no arguments are provided
    if( args[1] == NULL ) {
        for( int i = 0; i < ENVAMT; i++ ) {
            printENV(envList[i]);
        }
    }
    // Check if export param was entered correctly and if so overwrite the variable
    else {
        argCopy = args[1];

        // Separate name from the value
        token = strtok_r(argCopy, "=", &argCopy);

        // Search for name to overwrite
        for(int i = 0; i < ENVAMT; i++) {
            // Name found
            if(strcmp(token, envList[i].name) == 0) {
                nameFoundFlag = 1;
                // Get the new value
                token = strtok_r(argCopy, "=", &argCopy);

                // Free current env variable value if not empty
                if(envList[i].value != NULL) {
                    free(envList[i].value);
                }

                // Allocate and store new value in env variable
                envList[i].value = (char*)malloc( (strlen(token) + 1) * sizeof(char) );
                strcpy(envList[i].value, token);
            }
        }

    }
}

void printENV( env toPrint ) {
    printf("declare -x %s=\"%s\"\n", toPrint.name, toPrint.value);
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


//void freeArgs( char* args[], int numArgs ) {
void freeArgs( char* args[] ) {
    /*for( int i = 0; i < numArgs; i++ ) {
        free(args[i]);
    }*/
    if(args == NULL) {
        return;
    }
    int i = 0;

    while(args[i] != NULL) {
        free(args[i]);
        i++;
    }

    free(args);
}

void freeLineVariables( char ** args[2], char *outFile[2], char *inFile[2]) {
    // Free all variables used
    for( int i = 0; i < 2; i++){
        if(inFile[i] != NULL) {
            free(inFile[i]);
        }
        if(outFile[i] != NULL) {
            free(outFile[i]);
        }
        freeArgs(args[i]);
    }
}

void clearInputBuffer() {
    char c;
    do {
        c = getchar();
    } while( c != '\n' && c != EOF);
}
