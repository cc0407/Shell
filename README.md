# Shell in C
A homebrew shell written in C, to mimic the debian linux terminal.

# **Functions Implemented:**

## **exit**
- This function will kill any active subprocesses and then exit the shell
- The program identifies any active subprocesses by adding their PID to a linked list. On exit it will kill all PIDs present in the linked list
### Usage
- `exit [status]`
- `[status]` is an optional command to specify the exit code
### Assumptions
- exit must be the first argument presented in the line. Leading and trailing spaces do not matter
- If second argument exists it will always be converted to an int
### Limitations
- If second argument passed is not an int (i.e. "abc"), the shell will exit with a status of 0
### How it was tested
- This command was tested with (i.e. `exit 1`) and without (i.e. `exit`) arguments
- This command was tested with correct (i.e. `exit 1`) and incorrect (i.e. `exit abc`) arguments
- This command was tested as being the first argument on the line (i.e. `exit`) and not (i.e. `sort < temp.txt | exit`)

---
## **Commands with no arguments**
- The shell can accept any command without arguments
- If the command exists in the current working directory or in myPATH then it will run the command, otherwise it will print an error to the user and continue running
### Usage
- any command, i.e. `ls`, `sort`, etc
### Assumptions
- the function must be the first argument passed either in the line (i.e. `ls`) or after a pipe (i.e. `sort < temp.txt | ls`)
### Limitations
- The command will only run if it is present in either the current working directory, or in myPATH
### How it was tested
- This command was tested as being the first argument on the line (i.e. `ls`) and after a pipe (i.e. `sort < temp.txt | ls`)
- This command was tested with a command located in the current working directory (i.e. `./myShell`)
- This command was tested with a command located in myPATH (i.e. `ls`)
- This command was tested with a command not located in myPATH or in the current working directory (i.e. `abc`)

---
## **Commands with arguments**
- The shell can accept any command with arguments
- If the command exists in the current working directory or in myPATH then it will run the command, otherwise it will print an error to the user and continue running
### Usage
- any command, i.e. `ls -lta`, `sort temp.txt`, etc
### Assumptions
- the function must be the first argument passed either in the line (i.e. `ls`) or after a pipe (i.e. `ping -c 2 google.com | grep rtt`)
- The option parameters must be present on the line after the command
- The optional parameters and command must be separated by a space (i.e. `ls -lta` vs `ls-lta`)
### Limitations
- The command will only run if it is present in either the current working directory, or in myPATH
### How it was tested
- This command was tested as being the first argument on the line (i.e. `ls -lta`) and after a pipe (i.e. `ping -c 2 google.com | grep rtt`)
- This command was tested with a command located in myPATH (i.e. `ls -lta`)
- This command was tested with a command not located in myPATH or in the current working directory (i.e. `abc -d`)

---
## **Asynchronous commands with &**
- The shell will run any command asynchronously if & is present as the final argument on the line
- Trailing spaces after & do not matter and are ignored by the parser
### Usage
- any command, i.e. `ls &`, `sleep 5 &`, etc
### Assumptions and Limitations
- the `&` argument must be the final argument passed in on the line
### How it was tested
- This command was tested with a command that didnt have optional arguments (i.e. `ls &`)
- This command was tested with a command that had optional arguments (i.e. `sleep 5 &`)
- This command was tested with and without trailing spaces after &
- This command was tested when & was not the final argument on the line, it did not run asynchronously

---
## **Redirecting output to a file**
- The shell can redirect a commands output to a file using the `>` argument
### Usage
- `[command] [optional arguments] > [output filename]`
- `[command] [optional arguments]> [output filename]`
- `[command] [optional arguments] >[output filename]`
- `[command] [optional arguments]>[output filename]`
- Can include any combination of spaces or not before and after `>`
### Assumptions
- N/A
### Limitations
- `[output filename]` must be a valid filename or else the parser will return an error to the user
- There must only be one `>` present in a command or else the parser will return an error to the user
### How it was tested
- This command was tested with a command that didnt have optional arguments (i.e. `ls > test.txt`)
- This command was tested with a command that had optional arguments (i.e. `ls -lta > test.txt`)
- This command was tested with and without spaces around `>`
- This command was tested with multiple `>` keys in one command (i.e. `ls > test > test1`)
- This command was tested with a pipe (i.e. `ls | grep ./myShell > test.txt`)

---
## **Redirecting input from a file**
- The shell can redirect a commands input from a file using the `<` argument
### Usage
- `[command] [optional arguments] < [input filename]`
- `[command] [optional arguments]< [input filename]`
- `[command] [optional arguments] <[input filename]`
- `[command] [optional arguments]<[input filename]`
- Can include any combination of spaces or not before and after `<`
### Assumptions
- N/A
### Limitations
- `[input filename]` must be a valid filename or else the parser will return an error to the user
- `[input filename]` must be a valid file or else the parser will return an error to the user
- There must only be one `<` present in a command or else the parser will return an error to the user
### How it was tested
- This command was tested with a command that didnt have optional arguments (i.e. `sort < test.txt`)
- This command was tested with a command that had optional arguments (i.e. `ls -lta < test.txt`)
- This command was tested with and without spaces around `<`
- This command was tested with multiple `<` keys in one command (i.e. `sort < test.txt < test1.txt`)
- This command was tested with a pipe (i.e. `sort < test.txt | grep c`)

---
## **Piping**
- The shell can redirect a commands output to another commands input using the `|` argument
### Usage
- `[command] [optional arguments] | [command] [optional arguments]`
- `[command] [optional arguments]| [command] [optional arguments]`
- `[command] [optional arguments] |[command] [optional arguments]`
- `[command] [optional arguments]|[command] [optional arguments]`
- Can include any combination of spaces or not before and after `|`
### Assumptions
- The user only has access to one pipe in the line; cant pipe multiple times.
### Limitations
- There must only be one `|` present in a command or else the parser will return an error to the user
### How it was tested
- This command was tested with a command that didnt have optional arguments (i.e. `sort < test.txt | grep c`)
- This command was tested with a command that had optional arguments (i.e. `ls -lta | grep ./myShell`)
- This command was tested with and without spaces around `|`
- This command was tested with multiple `|` keys in one command (i.e. `sort < test.txt | grep c | grep c`)
- This command was tested with varying input and output redirections (i.e. `sort < test.txt | grep c`) (i.e. `sort < test.txt | grep c > test.txt`)
- The above command was tested with varying amounts of spaces between all characters (i.e. `sort<test.txt|grep c>test1.txt`)

---
## **Limited shell environment variables: $myPATH, $myHISTFILE, $myHOME**
- The shell stores values for the three variables `$myPATH`, `$myHISTFILE`, `$myHOME` and will replace the variable with its value if presented in the line to be parsed
### Usage
- `echo $myPATH`
### Assumptions
- The user must include `$` before the variable for it to be replaced by the parser
- `$myPATH` defaults to '/bin'
- If the user wants to include more than one path in $myPATH, they should be separated by `:`
- `$myHOME` defaults to the users home directory
- `$myHISTFILE` defaults to '~/CIS3110_history'
- If no history file is present, it will be created
### Limitations
- Restricted to only the three environment variables
### How it was tested
- This command was tested with a single replacement (i.e. `echo $myPATH`)
- This command was tested with a multiple replacements (i.e. `echo $myPATH $myHOME`)
- This command was tested with a variable that wasn't one of the three (i.e. `echo $myTEST`), did not replace $myTEST with anything

---
## **Reading in a profile file on initialization**
- The shell can be configured using a profile file located in the users home directory
- **A profile file is included in this directory, move it to your home directory if you would like to use it.**
### Usage
- Automatic on shell initialization
### Assumptions
- The profile file must be present as '~/.CIS3110_profile'
- The profile file can include any commands, including built-in commands
- The `$myPATH` variable is changed from its default in this file, so the user is able to enter more commands.
### Limitations
- Commands run in this profile do not appear in the history file, this is a design choice and not a programming limitation.
### How it was tested
- This command was tested when the profile file was present
- This command was tested when the profile file wasnt present
- This command was tested when the profile file contained invalid commands

---
## **export**
- This built-in command will either display all environment variables, or change a specified environment variable
### Usage
- `export` prints out all environment variables and their values
- `export [variable name]=[value]` modifies an environment variable
### Assumptions
- Any argument after `export [variable name]=[value]` is ignored by the parser
- If the user input a correct variable name but no value, an error will be returned to the user
### Limitations
- There must be no space between the variable name, `=`, and the value
- If the user enters a variable name that does not match any of the three environment variables, the command is ignored
### How it was tested
- This command was tested with no optional arguments (i.e. `export`)
- This command was tested with overwriting an environment variable (i.e. `export myPATH=/bin:/usr/bin`)
- This command was tested with overwriting a non-existent environment variable (i.e. `export myABC=123`)
- This command was tested with overwriting an environment variable with no value (i.e. `export myPATH=`)

---
## **history**
- This built-in command will either display all environment variables, or change a specified environment variable
### Usage
- `history` prints out all commands in the history file
- `history [n]` prints out the last n commands in the history file
### Assumptions
- If `[n]` is <= 0, then nothing is returned
- If `[n]` is greater than the total amount of lines in the history file, the entire history file is printed
- If `[n]` is non-numeric, 0 is used instead
### Limitations
- No error message is returned to the user if they enter a non-numeric value for `[n]`
### How it was tested
- This command was tested with no optional arguments (i.e. `history`)
- This command was tested with a numeric value `[n]` that is less than the total lines in the history file (i.e. `history 5`)
- This command was tested with a numeric value `[n]` that is greater than the total lines in the history file (i.e. `history 500`)
- This command was tested with a non-numeric value `[n]` (i.e. `history abc`)
- This command was tested with a numeric value `[n]` that is less than 0 (i.e. `history -5`)
- This command was tested with a numeric value `[n]` that is equal to 0 (i.e. `history 0`)

---
## **cd**
- This built-in command will change the current working directory
### Usage
- `cd` changes directory to $myHOME
- `cd [path]` changes directory to path
- `cd ..` moves up a directory
### Assumptions
- If `[path]` does not exist, an error is returned to the user
### Limitations
- N/A
### How it was tested
- This command was tested with no optional arguments (i.e. `cd`)
- This command was tested with a valid path (i.e. `cd $myHOME`)
- This command was tested with an invalid path (i.e. `cd abcdefghijk`)
- This command was tested with `..` (i.e. `cd ..` and `cd ../..`)
