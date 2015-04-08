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
    if (input[cursor] > 48 && input[cursor] < 57 && cursor < inputLength) {
        while (input[cursor] > 48 && input[cursor] < 57 && cursor < inputLength) {
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
    int line = 0;
    char lineContents[SCREEN_WIDTH];
    struct number parsedNumber;
    char buffer[MAX_LINES][SCREEN_WIDTH];
    
    while(1) {
        printf("%c", prompt);
        scanf("%s", commandStr);
        for (i = 0; i < strlen(commandStr);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
            command = commandStr[i];
            parsedNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
            strcpy(lineContents, buffer[line]);
            if (parsedNumber.value >= 0) { //Input is number
                i += parsedNumber.size;
                line = parsedNumber.value;
            } else {
                switch (command) {
                    case 'q':
                        exit(0); //Exits the program
                        break;
                    case 'n':
                        printf("N has been reached!\n");
                        break;
                    case 'e':
                        printf("%d\n", line);
                        break;
                    case 'c':
                        getchar(); //Flushes input buffer of newlines
                        fgets(lineContents, SCREEN_WIDTH, stdin);
                        printf("%d\n", line);
                        break;
                    case 'p':
                        printf("%s", lineContents);
                        break;
                    default:
                        printf("?\n");
                        break;
                }
                i++;
            }
            strcpy(buffer[line], lineContents);
        }
    }
    return 1;
}