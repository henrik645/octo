#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_COMMAND_SIZE 256
#define MAX_NUMBER_LEN 8
#define SCREEN_WIDTH 80

/* Declares a struct number with a value and the number of chars it took up in string form
 * for return from parseInt function.
 */
struct number {
    int value;
    int size;
};

/* Declares a struct range with a start and an end for ranging
 */
struct range {
    int start;
    int end;
};

struct number parseInt(char input[], int inputLength, int inputOffset);

/* Returns -1 if no integer was found and the integer if it was found (only positive values) 
 * It utilises the fact that in ASCII, numbers from 0 to 9 comes after each other.
 */
struct number parseInt(char input[], int inputLength, int inputOffset) {
    int cursor = inputOffset;
    int numCur = 0;
    char number[MAX_NUMBER_LEN] = {};
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

/* Parses a command and performs an action. Returns 1 when encountered with an error
 * Returns 0 when a quit command is reached
 */
int main(int argc, char *argv[]) {
    char command;
    char commandStr[MAX_COMMAND_SIZE];
    int i;
    int linesInputted;
    int line = 0; //Shifted by one (0 - line 1, 1 - line 2 ...)
    int lines = 0; //Total amount of lines, not shifted
    char lineContents[SCREEN_WIDTH];
    struct number parsedNumber;
    char *buffer = NULL;
    int isRange = 0;
    struct range range;
    char *newBuffer = NULL;
    int newLines = 0;
    int c;
    int x;
    char z;
    int fileExists;
    char prompt = ':';
    char templine[SCREEN_WIDTH]; //Temporary files for use in the transpose command
    char inputLine[SCREEN_WIDTH];
    char *input = NULL;
    char *newInput = NULL;
    int inputSize = 0;
    FILE *fp;
    
    if (argc < 1 || argc > 3) {
        printf("Usage: %s [filename] [prompt]\n", argv[0]);
    } else if (argc == 2 || argc == 3) {
        fileExists = 1;
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("%s: No such file or directory", argv[1]);
            return 1;
        }
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                newLines++;
            }
        }
        newLines++;
        fclose(fp);
        lines = newLines;
        
        printf("%d\n", newLines);
        newBuffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH);
        if (newBuffer == NULL) {
            printf("Error: out of memory");
            free(buffer);
            exit(2);
        }
        buffer = newBuffer;
        fp = fopen(argv[1], "r");
        x = 0; //Line counter
        z = 0; //Column counter
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                z = 0;
                x++;
            } else {
                *(buffer + (x * SCREEN_WIDTH) + z) = c;
                z++;
            }
        }
        fclose(fp);
    } else {
        fileExists = 0;
    }
    if (argc == 3) {
        prompt = argv[3][0];
    }
    
    while(1) {
        printf("%c", prompt);
        gets(commandStr);
        for (i = 0; i < strlen(commandStr);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
            command = commandStr[i];
            parsedNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
            if (parsedNumber.value >= 0) { //Input is number
                i += parsedNumber.size;
                if (commandStr[i] == ',') {
                    i++; //Removes the ','
                    struct number endNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
                    i = endNumber.size; //endNumber.size was already initialized to i beforehand
                    if (endNumber.value >= 0) {
                        isRange = 1;
                        range.start = parsedNumber.value - 1;
                        range.end = endNumber.value - 1;
                    } else {
                        printf("?\n");
                        break;
                    }
                } else {
                    isRange = 0;
                }
                line = parsedNumber.value - 1; //To account for the shifting (see above at initialization)
            } else {
                if (line + 1 > lines || line + 1 < 1) {
                    line = lines;
                }
                switch (command) {
                    case 'q':
                        free(buffer); //Frees the text buffer
                        exit(0); //Exits the program
                        break;
                    case 'n':
                        if (isRange == 1) {
                            if (range.start + 1 > lines || range.end + 1 > lines || range.start < 0 || range.end < 0) {
                                printf("?\n");
                            } else {
                                for (x = range.start; x <= range.end; x++) {
                                    printf("%d\t%s\n", x + 1, buffer + (x * SCREEN_WIDTH));
                                }
                            }
                        } else {
                            if (line + 1 > lines || line < 1) {
                                printf("?\n");
                            } else {
                                strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
                                printf("%d\t%s\n", line + 1, lineContents);
                            }
                        }
                        break;
                    case 'e':
                        if (lines > 0) {
                            printf("%d/%d\n", line + 1, lines);
                        } else {
                            printf("%d/%d\n", line, lines);
                        }
                        break;
                    case 'c':
                        if (line + 1 > lines) {
                            lines = line + 1;
                            newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char));
                            if (newBuffer == NULL) {
                                fprintf(stderr, "Error: out of memory");
                                free(buffer);
                                exit(2);
                            }
                            buffer = newBuffer;
                        }
                        fgets(lineContents, SCREEN_WIDTH, stdin);
                        strtok(lineContents, "\n"); //Removes the trailing newline
                        for (x = 0; x < SCREEN_WIDTH; x++) { //Clears the line
                            *(buffer + (line * SCREEN_WIDTH) + x) = 0;
                        }
                        strcpy(buffer + (line * SCREEN_WIDTH), lineContents);
                        break;
                    case 'p':
                        if (isRange) {
                            if (range.start + 1 > lines || range.end + 1 > lines || range.start < 0 || range.end < 0) {
                                printf("?\n");
                            } else {
                                isRange = 0;
                                int x;
                                for (x = range.start; x <= range.end; x++) {
                                    printf("%s\n", buffer + (x * SCREEN_WIDTH));
                                }
                            }
                        } else {
                            if (line + 1 > lines || line < 1) {
                                printf("?\n");
                            } else {
                                strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
                                printf("%s\n", lineContents);
                            }
                        }
                        break;
                    case 'i':
                        linesInputted = 0;
                        inputSize = 0;
                        while (1) { //Clears newline from stdin
                            c = getchar();
                            if (c != '\n') {
                                ungetc(c, stdin); //Puts the character back
                                break;
                            }
                        }
                        while (1) {
                            fgets(inputLine, SCREEN_WIDTH, stdin);
                            if (strcmp(inputLine, ".\n") == 0) {
                                break;
                            } else if (strcmp(inputLine, "\n") == 0) {
                                inputLine[0] = ' ';
                                inputLine[1] = '\0';
                            } else {
                                strtok(inputLine, "\n");
                            }
                            if (linesInputted + 1 > inputSize) {
                                inputSize = linesInputted + 1;
                                newInput = realloc(input, inputSize * SCREEN_WIDTH * sizeof(char));
                                if (newInput == NULL) {
                                    printf("Error: out of memory\n");
                                    exit(1);
                                }
                                input = newInput;
                            }
                            strcpy(input + (linesInputted * SCREEN_WIDTH * sizeof(char)), inputLine);
                            linesInputted++;
                        }
                        lines += linesInputted; //Adds to the buffer
                        newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Adds to the buffer
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error: out of memory");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        memmove(buffer + ((line + linesInputted) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH), (lines - line - linesInputted) * SCREEN_WIDTH * sizeof(char)); //Shifts the memory up x spaces (the number of lines entered)
                        for (x = 0; x < linesInputted; x++) {
                            strcpy(buffer + ((line + x) * SCREEN_WIDTH), (input + (x * SCREEN_WIDTH * sizeof(char))));
                        }
                        free(input); //Resets the input
                        input = NULL;
                        break;
                    case 'd':
                        if (isRange == 1) {
                            isRange = 0;
                            if (range.start + 1 <= lines && range.end + 1 <= lines && range.start + 1 >= 1 && range.end + 1 >= 1) {
                                memmove(buffer + (range.start * SCREEN_WIDTH), buffer + ((range.end + 1) * SCREEN_WIDTH), ((range.end - range.start) + 1) * SCREEN_WIDTH * sizeof(char)); //Plus one since this is an inclusive delete
                            }
                            lines -= range.end - range.start + 1;
                        } else {
                            if (line + 1 < lines) { //Perform only if this isn't the last line (otherwise there's nothing to be shifted down
                                memmove(buffer + (line * SCREEN_WIDTH), buffer + ((line + 1) * SCREEN_WIDTH), (lines - (line + 1)) * SCREEN_WIDTH * sizeof(char)); //Shifts down the memory
                            }
                            lines--; //Removes the upper lines
                        }
                        newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Deallocates the empty line
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error deleting line");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        break;
                    case 'a':
                        if (lines > 0) {
                            line++; //Makes everything operate on the second line
                        }
                        if (line + 1 > lines) {
                            lines = line;
                        }
                        linesInputted = 0;
                        inputSize = 0;
                        while (1) { //Clears newline from stdin
                            c = getchar();
                            if (c != '\n') {
                                ungetc(c, stdin); //Puts the character back
                                break;
                            }
                        }
                        while (1) {
                            fgets(inputLine, SCREEN_WIDTH, stdin);
                            if (strcmp(inputLine, ".\n") == 0) {
                                break;
                            }
                            if (strcmp(inputLine, "\n") == 0) {
                                inputLine[0] = ' ';
                                inputLine[1] = '\0';
                            } else {
                                strtok(inputLine, "\n");
                            }
                            if (linesInputted + 1 > inputSize) {
                                inputSize = linesInputted + 1;
                                newInput = realloc(input, inputSize * SCREEN_WIDTH * sizeof(char));
                                if (newInput == NULL) {
                                    printf("Error: out of memory\n");
                                    exit(1);
                                }
                                input = newInput;
                            }
                            strcpy(input + (linesInputted * SCREEN_WIDTH * sizeof(char)), inputLine);
                            linesInputted++;
                        }
                        lines += linesInputted; //Adds to the buffer
                        newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Adds to the buffer
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error: out of memory");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        memmove(buffer + ((line + linesInputted) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH), (lines - line - linesInputted) * SCREEN_WIDTH * sizeof(char)); //Shifts the memory up x spaces (the number of lines entered)
                        for (x = 0; x < linesInputted; x++) {
                            strcpy(buffer + ((line + x) * SCREEN_WIDTH), input + (x * SCREEN_WIDTH * sizeof(char)));
                        }
                        free(input);
                        input = NULL;
                        break;
                    case 'w':
                        if (fileExists == 1) {
                            fp = fopen(argv[1], "w");
                            for (x = 0; x < lines; x++) {
                                for (z = 0; z < SCREEN_WIDTH - 1; z++) {
                                    c = *(buffer + (x * SCREEN_WIDTH) + z);
                                    if (c != 0) {
                                        fputc(c, fp); //Outputs the character to file
                                    }
                                }
                                fputc('\n', fp); //Puts a newline after every line
                            }
                            fclose(fp);
                        } else {
                            printf("?\n");
                        }
                        break;
                    case 't':
                        if (line + 2 <= lines && line + 1 > 0) {
                            strcpy(templine, buffer + ((line + 1) * SCREEN_WIDTH));
                            strcpy(buffer + ((line + 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
                            strcpy(buffer + (line * SCREEN_WIDTH), templine);
                        } else {
                            printf("?\n");
                        }
                        break;
                    case 'T':
                        if (line + 1 <= lines && line > 1) {
                            strcpy(templine, buffer + ((line - 1) * SCREEN_WIDTH));
                            strcpy(buffer + ((line - 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
                            strcpy(buffer + (line * SCREEN_WIDTH), templine);
                        } else {
                            printf("?\n");
                        }
                        break;
                    case '@':
                        if (lines > 0) {
                            isRange = 1;
                            range.start = 0;
                            range.end = lines - 1;
                        } else {
                            line = 0;
                        }
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