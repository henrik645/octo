#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define MAX_COMMAND_SIZE 256
#define MAX_NUMBER_LEN 8
#define SCREEN_WIDTH 80
#define VERSION "0.2"

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

void printUsage(char *programName) {
    printf("Usage: %s [options] [filename]\n\n", programName);
    printf("Options:\n");
    printf(" -h: Displays help\n");
    printf(" -p: Sets prompt\n");
}

/* Parses a command and performs an action. Returns 1 when encountered with an error
 * Returns 0 when a quit command is reached
 */
int main(int argc, char *argv[]) {
    char option;
    char cmdopts[] = "p:h";
    opterr = 0;
    
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
    int z;
    int fileExists;
    char fileName[SCREEN_WIDTH];
    char prompt = ':';
    char templine[SCREEN_WIDTH]; //Temporary files for use in the transpose command
    char inputLine[SCREEN_WIDTH];
    char *input = NULL;
    char *newInput = NULL;
    int inputSize = 0;
    int fileLines = 0;
    int fileChars = 0;
    char error[SCREEN_WIDTH];
    char searchstr[SCREEN_WIDTH];
    char replacestr[SCREEN_WIDTH];
    char *replaceptr; //For use by the search & replace command
    int unsaved = 0; //Set to 1 when there are unsaved changes
    char *copied = NULL; //Pointer to memory where copied sections of text are stored
    int copyLines = 0; //How many lines are stored there
    char copyLine[SCREEN_WIDTH];
    strcpy(error, "");
    FILE *fp;
    
    printf("octo v%s\n", VERSION);
    
    while ((option = getopt(argc, argv, cmdopts)) != -1) {
        switch (option) {
            case 'p':
                prompt = optarg[0];
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr, "Error: 'p' requires an argument.\n");
                    return 1;
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: Unknown option -%c\n", optopt);
                    return 1;
                } else {
                    fprintf(stderr, "Error: Unknown option.");
                    return 1;
                }
                break;
            default:
                abort();
        }
    }
    
    if (argc - optind > 1) { //Too many arguments
        printUsage(argv[0]);
        exit(1);
    } else if (optind + 1 == argc) {
        fileExists = 1;
        strcpy(fileName, argv[optind]);
        fp = fopen(fileName, "r");
        if (fp == NULL) {
            printf("new file\n");
        } else {
            while ((c = fgetc(fp)) != EOF) {
                if (c == '\n') {
                    newLines++;
                }
            }
            rewind(fp);
            lines = newLines;
            
            newBuffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH);
            if (newBuffer == NULL) {
                printf("Error: out of memory");
                free(buffer);
                exit(2);
            }
            buffer = newBuffer;
            x = 0; //Line counter
            z = 0; //Column counter
            while ((c = fgetc(fp)) != EOF) {
                if (c == '\n') {
                    z = 0;
                    x++;
                } else {
                    *(buffer + (x * SCREEN_WIDTH) + z) = c;
                    z++;
                    fileChars++;
                }
            }
            fclose(fp);
            printf("%d\n", fileChars);
        }
    } else {
        fileExists = 0;
    }
    
    while(1) {
        printf("%c", prompt);
        fgets(commandStr, MAX_COMMAND_SIZE, stdin);
        strtok(commandStr, "\n"); //Strips the newline away
        for (i = 0; i < strlen(commandStr);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
            command = commandStr[i];
            parsedNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
            if (parsedNumber.value >= 0) { //Input is number
                i = parsedNumber.size; //parsedNumber.size was already initialized to i beforehand
                if (commandStr[i] == ',') {
                    i++; //Removes the ','
                    struct number endNumber = parseInt(commandStr, MAX_NUMBER_LEN, i);
                    i = endNumber.size; //endNumber.size was already initialized to i beforehand
                    if (endNumber.value >= 0) {
                        if (parsedNumber.value > 0 && parsedNumber.value < lines && endNumber.value > 0 && endNumber.value < lines) {
                            isRange = 1;
                            range.start = parsedNumber.value - 1;
                            range.end = endNumber.value - 1;
                        } else {
                            printf("?\n");
                            strcpy(error, "range limits out of range");
                        }
                    } else {
                        strcpy(error, "wrongly formatted range");
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
                        if (unsaved == 0) {
                            free(buffer); //Frees the text buffer
                            exit(0); //Exits the program
                        } else {
                            printf("!\n");
                            strcpy(error, "unsaved changes");
                        }
                        break;
                    case 'n':
                        if (isRange == 1) {
                            if (range.start + 1 > lines || range.end + 1 > lines || range.start < 0 || range.end < 0) {
                                strcpy(error, "range outside limits");
                                printf("?\n");
                            } else {
                                for (x = range.start; x <= range.end; x++) {
                                    printf("%d\t%s\n", x + 1, buffer + (x * SCREEN_WIDTH));
                                }
                            }
                        } else {
                            if (line + 1 > lines || line < 1) {
                                strcpy(error, "line entered is outside limits");
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
                        if (strcmp(lineContents, "\n") == 0) {
                            strcpy(lineContents, "");
                        }
                        strtok(lineContents, "\n"); //Removes the trailing newline
                        for (x = 0; x < SCREEN_WIDTH; x++) { //Clears the line
                            *(buffer + (line * SCREEN_WIDTH) + x) = 0;
                        }
                        strcpy(buffer + (line * SCREEN_WIDTH), lineContents);
                        break;
                    case 'p':
                        if (isRange) {
                            if (range.start + 1 > lines || range.end + 1 > lines || range.start < 0 || range.end < 0) {
                                strcpy(error, "range outside limits");
                                printf("?\n");
                            } else {
                                int x;
                                for (x = range.start; x <= range.end; x++) {
                                    printf("%s\n", buffer + (x * SCREEN_WIDTH));
                                }
                            }
                        } else {
                            if (line + 1 > lines || line < 1) {
                                strcpy(error, "line entered is outside limits");
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
                                inputLine[0] = '\0';
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
                        unsaved = 1;
                        break;
                    case 'd':
                        if (isRange == 1) {
                            isRange = 0;
                            if (range.start + 1 <= lines && range.end + 1 <= lines && range.start + 1 >= 1 && range.end + 1 >= 1) {
                                memmove(buffer + (range.start * SCREEN_WIDTH), buffer + ((range.end + 1) * SCREEN_WIDTH), ((range.end - range.start) + 1) * SCREEN_WIDTH * sizeof(char)); //Plus one since this is an inclusive delete
                            }
                            if (lines - (range.end - range.start) + 1 > 0) {
                                lines -= range.end - range.start + 1;
                            } else {
                                lines = 0;
                            }
                        } else {
                            if (line + 1 < lines) { //Perform only if this isn't the last line (otherwise there's nothing to be shifted down
                                memmove(buffer + (line * SCREEN_WIDTH), buffer + ((line + 1) * SCREEN_WIDTH), (lines - (line + 1)) * SCREEN_WIDTH * sizeof(char)); //Shifts down the memory
                            }
                            if (lines > 0) {
                                lines--; //Removes the upper lines
                            } else {
                                lines = 0;
                            }
                        }
                        newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Deallocates the empty line
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error deleting line");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        unsaved = 1;
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
                                inputLine[0] = '\0';
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
                        unsaved = 1;
                        break;
                    case 'w':
                        fileChars = 0;
                        if (fileExists == 1) {
                            fp = fopen(fileName, "w");
                            for (x = 0; x < lines; x++) {
                                for (z = 0; z < SCREEN_WIDTH - 1; z++) {
                                    c = *(buffer + (x * SCREEN_WIDTH) + z);
                                    if (c != 0) {
                                        fputc(c, fp); //Outputs the character to file
                                        fileChars++;
                                    }
                                }
                                fputc('\n', fp); //Puts a newline after every line
                            }
                            fclose(fp);
                            printf("%d\n", fileChars);
                            unsaved = 0;
                        } else {
                            printf("File: ");
                            fgets(fileName, SCREEN_WIDTH, stdin);
                            strtok(fileName, "\n"); //Removes the trailing newline
                            fp = fopen(fileName, "w");
                            if (fp == NULL) {
                                printf("!");
                                strcpy(error, "file not found");
                                fileExists = 0;
                            } else {
                                fileExists = 1;
                                for (x = 0; x < lines; x++) {
                                    for (z = 0; z < SCREEN_WIDTH - 1; z++) {
                                        c = *(buffer + (x * SCREEN_WIDTH) + z);
                                        if (c != 0) {
                                            fputc(c, fp);
                                            fileChars++;
                                        }
                                    }
                                    fputc('\n', fp);
                                }
                                fclose(fp);
                                printf("%d\n", fileChars);
                                unsaved = 0;
                            }
                        }
                        break;
                    case 'W':
                        printf("File: ");
                        fgets(fileName, SCREEN_WIDTH, stdin);
                        strtok(fileName, "\n"); //Removes the trailing newline
                        fp = fopen(fileName, "w");
                        if (fp == NULL) {
                            printf("!\n");
                            strcpy(error, "file not found");
                            strcpy(fileName, "\0"); //Empties the filename
                        } else {
                            fileExists = 1;
                            fileChars = 0;
                            for (x = 0; x < lines; x++) {
                                for (z = 0; z < SCREEN_WIDTH - 1; z++) {
                                    c = *(buffer + (x * SCREEN_WIDTH) + z);
                                    if (c != 0) {
                                        fputc(c, fp);
                                        fileChars++;
                                    }
                                }
                                fputc('\n', fp);
                            }
                            fclose(fp);
                            printf("%d\n", fileChars);
                            unsaved = 0;
                        }
                        break;
                    case 'o':
                        if (unsaved == 0) {
                            printf("File: ");
                            fgets(fileName, SCREEN_WIDTH, stdin);
                            strtok(fileName, "\n"); //Removes the trailing newline
                            fp = fopen(fileName, "r");
                            if (fp == NULL) {
                                printf("!\n");
                                strcpy(error, "file not found");
                                fileExists = 0;
                            } else {
                                fileExists = 1;
                                fileLines = 0;
                                while ((c = fgetc(fp)) != EOF) { //Counts the file size for sizing of the buffer
                                    if (c == '\n') {
                                        fileLines++;
                                    }
                                }
                                lines = fileLines;
                                newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Reallocates the buffer to the desired size
                                if (newBuffer == NULL) {
                                    fprintf(stderr, "Error: out of memory\n");
                                    fclose(fp);
                                    exit(1);
                                }
                                buffer = newBuffer;
                                rewind(fp); //Rewinds the file for reading actual file contents
                                x = 0;
                                z = 0;
                                fileChars = 0;
                                while ((c = fgetc(fp)) != EOF) {
                                    if (c == '\n') {
                                        z = 0;
                                        x++;
                                    } else {
                                        *(buffer + (x * SCREEN_WIDTH) + z) = c;
                                        z++;
                                        fileChars++;
                                    }
                                }
                                fclose(fp);
                                printf("%d\n", fileChars);
                            }
                        } else {
                            printf("!\n");
                            strcpy(error, "unsaved changes");
                        }
                        break;
                    case 't':
                        if (line + 2 <= lines && line + 1 > 0) {
                            strcpy(templine, buffer + ((line + 1) * SCREEN_WIDTH));
                            strcpy(buffer + ((line + 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
                            strcpy(buffer + (line * SCREEN_WIDTH), templine);
                        } else {
                            strcpy(error, "can't transpose last line");
                            printf("?\n");
                        }
                        unsaved = 1;
                        break;
                    case 'T':
                        if (line + 1 <= lines && line > 1) {
                            strcpy(templine, buffer + ((line - 1) * SCREEN_WIDTH));
                            strcpy(buffer + ((line - 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
                            strcpy(buffer + (line * SCREEN_WIDTH), templine);
                        } else {
                            strcpy(error, "can't transpose first line");
                            printf("?\n");
                        }
                        break;
                    case 'h':
                        if (error[0] == '\0') {
                            strcpy(error, "no error found");
                            printf("?\n");
                        } else {
                            printf("%s\n", error);
                        }
                        unsaved = 1;
                        break;
                    case 'f':
                        printf("Search: ");
                        fgets(searchstr, SCREEN_WIDTH, stdin);
                        strtok(searchstr, "\n"); //Removes trailing newline
                        if (isRange == 1) {
                            if (range.start >= 0 && range.start < lines && range.end >= 0 && range.end < lines) {
                                for (x = range.start; x <= range.end; x++) {
                                    if (strstr(buffer + (x * SCREEN_WIDTH), searchstr) != NULL) {
                                        printf("%d\t%s\n", x + 1, buffer + (x * SCREEN_WIDTH));
                                    }
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "lines out of range");
                            }
                        } else {
                            if (line >= 0 && line < lines) {
                                if (strstr(buffer + (line * SCREEN_WIDTH), searchstr) != NULL) { //match was found
                                    printf("%d\t%s\n", line + 1, buffer + (line * SCREEN_WIDTH));
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "line out of range");
                            }
                        }
                        break;
                    case 's':
                        printf("Search: ");
                        fgets(searchstr, SCREEN_WIDTH, stdin);
                        strtok(searchstr, "\n");
                        printf("Replace: ");
                        fgets(replacestr, SCREEN_WIDTH, stdin);
                        strtok(replacestr, "\n");
                        if (isRange == 1) {
                            if (range.start >= 0 && range.start < lines && range.end >= 0 && range.end < lines) {
                                for (x = range.start; x <= range.end; x++) {
                                    if ((replaceptr = strstr(buffer + (x * SCREEN_WIDTH), searchstr)) != NULL) { //match was found
                                        for (z = 0; z <= strlen(replacestr); z++) {
                                            *(replaceptr + z) = replacestr[z];
                                        }
                                    }
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "lines out of range");
                            }
                        } else {
                            if (line >= 0 && line < lines) {
                                if ((replaceptr = strstr(buffer + (line * SCREEN_WIDTH), searchstr)) != NULL) {
                                    for (x = 0; x <= strlen(replacestr); x++) {
                                        *(replaceptr + x) = replacestr[x];
                                    }
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "line out of range");
                            }
                        }
                        unsaved = 1;
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
                    case '!':
                        unsaved = 0;
                        break;
                    case 'z':
                        free(copied);
                        copyLines = 0;
                        copied = NULL;
                        if (isRange == 1) {
                            if (range.start >= 0 && range.start < lines && range.end >= 0 && range.end < lines) {
                                for (x = range.start; x <= range.end; x++) {
                                    strcpy(copyLine, buffer + (x * SCREEN_WIDTH)); //Copies one line at a time to copyLine
                                    newBuffer = realloc(copied, (copyLines + 1) * SCREEN_WIDTH * sizeof(char));
                                    if (newBuffer == NULL) {
                                        printf("Error: out of memory\n");
                                        exit(1);
                                    }
                                    copied = newBuffer;
                                    strcpy(copied + (copyLines * SCREEN_WIDTH), copyLine);
                                    copyLines++;
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "lines out of range");
                            }
                        } else {
                            if (line >= 0 && line < lines) {
                                newBuffer = realloc(copied, SCREEN_WIDTH * sizeof(char));
                                if (newBuffer == NULL) {
                                    printf("Error: out of memory\n");
                                    exit(1);
                                }
                                copied = newBuffer;
                                strcpy(copied, buffer + (line * SCREEN_WIDTH));
                                copyLines = 1;
                            } else {
                                printf("?\n");
                                strcpy(error, "line out of range");
                            }
                        }
                        break;
                    case 'x':
                        free(copied);
                        copyLines = 0;
                        copied = NULL;
                        if (isRange == 1) {
                            if (range.start >= 0 && range.start < lines && range.end >= 0 && range.end < lines) {
                                for (x = range.start; x <= range.end; x++) {
                                    strcpy(copyLine, buffer + (x * SCREEN_WIDTH)); //Copies one line at a time to copyLine
                                    newBuffer = realloc(copied, (copyLines + 1) * SCREEN_WIDTH * sizeof(char));
                                    if (newBuffer == NULL) {
                                        printf("Error: out of memory\n");
                                        exit(1);
                                    }
                                    copied = newBuffer;
                                    strcpy(copied + (copyLines * SCREEN_WIDTH), copyLine);
                                    copyLines++;
                                }
                            } else {
                                printf("?\n");
                                strcpy(error, "lines out of range");
                            }
                        } else {
                            if (line >= 0 && line < lines) {
                                newBuffer = realloc(copied, SCREEN_WIDTH * sizeof(char));
                                if (newBuffer == NULL) {
                                    printf("Error: out of memory\n");
                                    exit(1);
                                }
                                copied = newBuffer;
                                strcpy(copied, buffer + (line * SCREEN_WIDTH));
                                copyLines = 1;
                            } else {
                                printf("?\n");
                                strcpy(error, "line out of range");
                            }
                        }
                        
                        /* Copy into buffer above
                         * Delete lines below (cut from delete command)
                         */
                         
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
                            fprintf(stderr, "Error cutting line");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        unsaved = 1;
                        break;
                    case 'v':
                        lines += copyLines; //Adds the newly copied lines
                        newBuffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Adds to the buffer
                        if (newBuffer == NULL) {
                            fprintf(stderr, "Error: out of memory");
                            free(buffer);
                            exit(2);
                        }
                        buffer = newBuffer;
                        memmove(buffer + ((line + copyLines) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH), (lines - line - copyLines) * SCREEN_WIDTH * sizeof(char)); //Shifts the memory up x spaces (the number of lines entered)
                        for (x = 0; x < copyLines; x++) {
                            strcpy(buffer + ((line + x) * SCREEN_WIDTH), copied + (x * SCREEN_WIDTH));
                        }
                        unsaved = 1;
                        break;
                    case '\t':
                        break;
                    case ' ':
                        break;
                    case '\n':
                        break;
                    default:
                        printf("?\n");
                        strcpy(commandStr, ""); //Empties commandStr, accepting no more commands after an error
                        strcpy(error, "unknown command");
                        break;
                }
                i++;
            }
        }
    }
    return 1;
}