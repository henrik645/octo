#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_COMMAND_SIZE 256
#define MAX_NUMBER_LEN 8
#define SCREEN_WIDTH 80
#define MAX_LINES 50

/* Declares a struct number with a value and the number of chars it took up in string form
 * for return from parseInt function.
 */
struct number {
    int value;
    int size;
};

struct range {
    int start;
    int end;
};

struct number parseInt(char input[], int inputLength, int inputOffset);
int parseCommands(char prompt);

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        printf("Usage: %s filename [prompt]\n", argv[0]);
    } else {
        FILE *fp = fopen(argv[1], "r");
        if (fp == 0) {
            printf("%s: No such file or directory\n", argv[1]);
        } else {
            char prompt;
            if (argc == 3) {
                prompt = *argv[2];
            } else {
                prompt = ':';
            }
            parseCommands(prompt);
        }
    }
    return 0;
}

/* Returns -1 if no integer was found and the integer if it was found (only positive values) 
 * It uses the fact that in ASCII, numbers from 0 to 9 comes after each other.
 */
struct number parseInt(char input[], int inputLength, int inputOffset) {
    int cursor = inputOffset;
    int numCur = 0;
    char number[8] = {};
    if (input[cursor] >= 48 && input[cursor] <= 57 && cursor < inputLength) {
        while (input[cursor] >= 48 && input[cursor] <= 57 && cursor < inputLength) {
            number[numCur] = input[cursor];
            numCur++;
            cursor++;
        }
        struct number integer;
        integer.value = atoi(number);
        integer.size = cursor;
        return integer;
    } else {
        struct number integer;
        integer.value = -1;
        integer.size = 0;
        return integer;
    }
}

/* Parses a command and performs an action. Returns 0 when encountered with an error
 * Returns 1 when the parsing and execution of the command is done
 */
int parseCommands(char prompt) {
    char command;
    char commandStr[MAX_COMMAND_SIZE];
    int i;
    int linesInputted;
    int line = 0;
    int lines = 0;
    char lineContents[SCREEN_WIDTH];
    struct number parsedNumber;
    //char buffer[MAX_LINES][SCREEN_WIDTH];
    //char *buffer = (char *) malloc(SCREEN_WIDTH * sizeof(char));
    char *buffer = NULL;
    int isRange = 0;
    struct range range;
    char *newBuffer;
    
    while(1) {
        printf("%c", prompt);
        scanf("%s", commandStr);
        for (i = 0; i < strlen(commandStr);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
            command = commandStr[i];
            parsedNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
            if (parsedNumber.value >= 0) { //Input is number
                i += parsedNumber.size;
                if (commandStr[i] == ',') {
                    i++; //Removes the ','
                    struct number endNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
                    i += endNumber.size - 2; //Minus two to account for a shift of one in parseInt
                    if (endNumber.value >= 0) {
                        isRange = 1;
                        range.start = parsedNumber.value;
                        range.end = endNumber.value;
                    } else {
                        printf("?\n");
                        break;
                    }
                } else {
                    isRange = 0;
                }
                line = parsedNumber.value;
            } else {
                char input[MAX_LINES][SCREEN_WIDTH];
                char inputLine[SCREEN_WIDTH];
                int x;
                switch (command) {
                    case 'q':
                        free(buffer); //Frees the text buffer
                        exit(1); //Exits the program
                        break;
                    case 'n':
                        printf("N has been reached!\n");
                        break;
                    case 'e':
                        printf("%d\n", line);
                        break;
                    case 'c':
                        if (line > lines) {
                            lines = line;
                            newBuffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char));
                            if (newBuffer == NULL) {
                                fprintf(stderr, "Error: out of memory");
                                free(buffer);
                                exit(2);
                            }
                            buffer = newBuffer;
                        }
                        getchar(); //Flushes input buffer of newlines
                        fgets(lineContents, SCREEN_WIDTH, stdin);
                        strtok(lineContents, "\n"); //Removes the trailing newline
                        strcpy(buffer + (line * SCREEN_WIDTH), lineContents);
                        break;
                    case 'p':
                        if (isRange) {
                            isRange = 0;
                            int x;
                            for (x = range.start; x <= range.end; x++) {
                                printf("%s\n", buffer + (x * SCREEN_WIDTH));
                            }
                        } else {
                            strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
                            printf("%s\n", lineContents);
                        }
                        break;
                    case 'i':
                        linesInputted = 0;
                        while (1) {
                            char c;
                            while (1) { //Clears newline from stdin
                                c = getchar();
                                if (c != '\n') {
                                    ungetc(c, stdin); //Puts the character back
                                    break;
                                }
                            }
                            fgets(inputLine, SCREEN_WIDTH, stdin);
                            if (strcmp(inputLine, ".\n") == 0) {
                                break;
                            } else {
                                strtok(inputLine, "\n");
                                strcpy(input[linesInputted], inputLine);
                                linesInputted++;
                            }
                        }
                        if (line > lines) {
                            lines = line;
                        }
                        lines += linesInputted; //Adds to the buffer
                        newBuffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Adds to the buffer
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error: out of memory");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        memmove(buffer + ((line + linesInputted) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH), (lines - line) * SCREEN_WIDTH * sizeof(char)); //Shifts the memory up x spaces (the number of lines entered)
                        for (x = 0; x < linesInputted; x++) {
                            strcpy(buffer + ((line + x) * SCREEN_WIDTH), input[x]);
                        }
                        break;
                    case 'd':
                        if (line < lines ) { //Perform only if this isn't the last line (otherwise there's nothing to be shifted down
                            memmove(buffer + (line * SCREEN_WIDTH), buffer + ((line + 1) * SCREEN_WIDTH), (lines - line) * SCREEN_WIDTH * sizeof(char)); //Shifts down the memory
                        }
                        lines--; //Removes the upper lines
                        newBuffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Deallocates the empty line
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error deleting line");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        break;
                    default:
                        printf("?\n");
                        break;
                }
                i++;
            }
        }
    }
    return 1;
}