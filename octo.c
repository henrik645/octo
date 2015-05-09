#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define MAX_COMMAND_SIZE 256
#define MAX_NUMBER_LEN 8
#define SCREEN_WIDTH 80
#define VERSION "0.3"
#define NEW_FILE "new file\n"
#define SURROUND 10
/* The number of lines in each direction the surround command prints out */

int lines = 0; //Total amount of lines, not shifted
int line = 0; //Shifted by one (0 - line 1, 1 - line 2 ...)
char error[SCREEN_WIDTH];
char *buffer = NULL;
char *new_buffer = NULL;
int unsaved = 0; //Set to 1 when there are unsaved changes
int file_exists = 0;
char file_name[SCREEN_WIDTH];
int isRange = 0;
struct range range;
char *copied = NULL; //Pointer to memory where copied sections of text are stored
int copy_lines = 0; //How many lines are stored there

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
    printf("Usage: %s [options] [file_name]\n\n", programName);
    printf("Options:\n");
    printf(" -h: Displays help\n");
    printf(" -p: Sets prompt\n");
    printf(" -v: Displays version\n");
}

void *update_buffer(void *buf, size_t size) {
    void *new_buf = realloc(buf, size);
    if (new_buf == NULL) {
        fprintf(stderr, "Error: not enough memory");
        free(buf);
        exit(2);
    }
    buf = new_buf;
    return buf;
}

void print_numbered_line(int line) {
    char lineContents[SCREEN_WIDTH];
    if (line + 1 > lines || line + 1 < 1) {
        strcpy(error, "line entered is outside limits");
        printf("?\n");
    } else {
        strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
        printf("%d\t%s\n", line + 1, lineContents);
    }
}

void print_numbered_lines(int start, int end) {
    int i;
    if (start + 1 > lines || end + 1 > lines || start < 0 || end < 0) {
        strcpy(error, "range outside limits");
        printf("?\n");
    } else {
        for (i = start; i <= end; i++) {
            print_numbered_line(i);
        }
    }
}

void quit_program(void) {
    if (unsaved == 0) {
        free(buffer); //Frees the text buffer
        exit(0); //Exits the program
    } else {
        printf("!\n");
        strcpy(error, "unsaved changes");
    }
}

void print_current_line(int line) {
    if (lines > 0) {
        printf("%d/%d\n", line + 1, lines);
    } else {
        printf("%d/%d\n", line, lines);
    }
}

void change_line(int line) {
    char lineContents[SCREEN_WIDTH];
    int i;
    
    if (line + 1 > lines) {
        lines = line + 1;
        new_buffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char));
        if (new_buffer == NULL) {
            fprintf(stderr, "Error: out of memory");
            free(buffer);
            exit(2);
        }
        buffer = new_buffer;
    }
    fgets(lineContents, SCREEN_WIDTH, stdin);
    if (strcmp(lineContents, "\n") == 0) {
        strcpy(lineContents, "");
    }
    strtok(lineContents, "\n"); //Removes the trailing newline
    for (i = 0; i < SCREEN_WIDTH; i++) { //Clears the line
        *(buffer + (line * SCREEN_WIDTH) + i) = 0;
    }
    strcpy(buffer + (line * SCREEN_WIDTH), lineContents);
    unsaved = 1;
}

void print_line(int line) {
    char lineContents[SCREEN_WIDTH];
    
    if (line + 1 > lines || line + 1 < 1) {
        strcpy(error, "line entered is outside limits");
        printf("?\n");
    } else {
        strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
        printf("%s\n", lineContents);
    }
}

void print_lines(int start, int end) {
    int i;
    
    if (start + 1 > lines || end + 1 > lines || start < 0 || end < 0) {
        strcpy(error, "range outside limits");
        printf("?\n");
    } else {
        for (i = start; i <= end; i++) {
            print_line(i);
        }
    }
}

void insert_line(char line[SCREEN_WIDTH], int current_line) {
    lines++;
    buffer = update_buffer(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char));
    memmove(buffer + ((current_line + 1) * SCREEN_WIDTH), buffer + (current_line * SCREEN_WIDTH), (lines - current_line - 1) * SCREEN_WIDTH * sizeof(char)); //Shifts the memory up x spaces (the number of lines entered)
    strcpy(buffer + current_line * SCREEN_WIDTH, line);
    
    new_buffer = realloc(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Deallocates the empty line
    if (new_buffer == NULL) {
        fprintf(stderr, "Error deleting line");
        free(buffer);
        exit(2);
    }
    buffer = new_buffer;
    
    unsaved = 1;
}

void insert_lines(int current_line) {
    int lines_inputted = 0;
    int input_size = 0;
    char input_line[SCREEN_WIDTH];
    char c;
    char *newInput;
    
    while (1) { //Clears newline from stdin
        c = getchar();
        if (c != '\n') {
            ungetc(c, stdin); //Puts the character back
            break;
        }
    }
    
    while (1) {
        fgets(input_line, SCREEN_WIDTH, stdin);
        
        if (strcmp(input_line, ".\n") == 0) {
            break;
        } else if (strcmp(input_line, "\n") == 0) {
            input_line[0] = '\0';
        } else {
            strtok(input_line, "\n");
        }
        insert_line(input_line, current_line + lines_inputted);
        lines_inputted++;
    }
}

void delete_line(int line) {
    if (line + 1 < lines) { //Perform only if this isn't the last line (otherwise there's nothing to be shifted down
        memmove(buffer + (line * SCREEN_WIDTH), buffer + ((line + 1) * SCREEN_WIDTH), (lines - (line + 1)) * SCREEN_WIDTH * sizeof(char)); //Shifts down the memory
    }
    if (lines > 0) {
        lines--; //Removes the upper lines
    } else {
        lines = 0;
    }
    unsaved = 1;
}

void delete_lines(int start, int end) {
    int i;
    
    if (start + 1 <= lines && end + 1 <= lines && start + 1 >= 1 && end + 1 >= 1) {
        for (i = start; i <= end; i++) {
            delete_line(i);
        }
    }
}

int write_file_name(char file_name[SCREEN_WIDTH]) {
    int i = 0;
    int x = 0;
    int file_chars = 0;
    char c;
    
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        strcpy(error, "could not open file for writing");
        return 1;
    }
    
    fp = fopen(file_name, "w");
    for (i = 0; i < lines; i++) {
        for (x = 0; x < SCREEN_WIDTH - 1; x++) {
            c = *(buffer + (i * SCREEN_WIDTH) + x);
            if (c == 0) {
                break;
            } else {
                fputc(c, fp); //Outputs the character to file
                file_chars++;
            }
        }
        fputc('\n', fp); //Puts a newline after every line
    }
    fclose(fp);
    printf("%d\n", file_chars);
    unsaved = 0;
    
    return 0;
}

int write_file() {
    int file_chars = 0;
    
    if (file_exists) {
        return write_file_name(file_name);
    }

    printf("File: ");
    fgets(file_name, SCREEN_WIDTH, stdin);
    strtok(file_name, "\n"); //Removes the trailing newline
    
    return write_file_name(file_name);
}

void open_file() {
    int file_lines = 0;
    int file_chars = 0;
    int c; // int needed for proper handling of EOF
    int y = 0;
    int x = 0;
    int longest_line = 0;
    char file_name[SCREEN_WIDTH];
    FILE *fp;
    
    printf("File: ");
    fgets(file_name, SCREEN_WIDTH, stdin);
    strtok(file_name, "\n"); //Removes the trailing newline
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("!\n");
        strcpy(error, "file not found");
        file_exists = 0;
    } else {
        file_exists = 1;
        while ((c = fgetc(fp)) != EOF) { //Counts the file sixe for sizing of the buffer
            if (c == '\n') {
                file_lines++;
                if (file_chars > longest_line) {
                    longest_line = file_chars;
                }
            } else {
                file_chars++;
            }
        }

        file_chars = 0;
        
        if (longest_line > SCREEN_WIDTH) {
            file_exists = 0;
            strcpy(error, "file is wider than screen width");
            printf("!\n");
        } else {
            lines = file_lines;
            buffer = update_buffer(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Reallocates the buffer to the desired size
            rewind(fp); //Rewinds the file for reading actual file contents
            
            while ((c = fgetc(fp)) != EOF) {
                if (c == '\n') {
                    x = 0;
                    y++;
                } else {
                    *(buffer + (y * SCREEN_WIDTH) + x) = c;
                    x++;
                    file_chars++;
                }
            }
            fclose(fp);
            printf("%d\n", file_chars);
        }
    }
}

void transpose_next(int line) {
    char templine[SCREEN_WIDTH];
    
    if (line + 2 <= lines && line + 1 > 0) {
        strcpy(templine, buffer + ((line + 1) * SCREEN_WIDTH));
        strcpy(buffer + ((line + 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
        strcpy(buffer + (line * SCREEN_WIDTH), templine);
    } else {
        strcpy(error, "can't transpose last line");
        printf("?\n");
    }
    unsaved = 1;
}

void transpose_previous(int line) {
    char templine[SCREEN_WIDTH];
    
    if (line + 1 <= lines && line + 1 >= 1) {
        strcpy(templine, buffer + ((line - 1) * SCREEN_WIDTH));
        strcpy(buffer + ((line - 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
        strcpy(buffer + (line * SCREEN_WIDTH), templine);
    } else {
        strcpy(error, "can't transpose first line");
        printf("?\n");
    }
}

void print_help() {
    if (error[0] == '\0') {
        strcpy(error, "no error found");
        printf("?\n");
    } else {
        printf("%s\n", error);
    }
}

int find_in_line(int line, char searchstr[SCREEN_WIDTH]) {
    if (line >= 0 && line < lines) {
        if (strstr(buffer + (line * SCREEN_WIDTH), searchstr) != NULL) { //match was found
            return 1;
        }
    } else {
        printf("?\n");
        strcpy(error, "line out of range");
    }
    return 0;
}

void find_in_range(int start, int stop, char searchstr[SCREEN_WIDTH]) {
    int i = 0;

    for (i = start; i <= stop; i++) {
        if (find_in_line(i, searchstr)) {
            printf("%d\t%s\n", i + 1, buffer + (i * SCREEN_WIDTH));
        }
    }
}

void search_replace(int line, char searchstr[SCREEN_WIDTH], char replacestr[SCREEN_WIDTH]) {
    int i = 0;
    int lineoffset = 0;
    int extralength = 0;
    char *replaceptr = NULL;
    int bytes;
    int to;
    int from;
    
    if (line >= 0 && line < lines) {
        while (1) {
            if ((replaceptr = strstr(buffer + (line * SCREEN_WIDTH) + lineoffset, searchstr)) != NULL) {
                extralength = strlen(replacestr) - strlen(searchstr);
                
                bytes = SCREEN_WIDTH - (((replaceptr + strlen(searchstr)) - buffer) % SCREEN_WIDTH) - extralength;
                to = (replaceptr + strlen(searchstr) - buffer) % SCREEN_WIDTH;
                from = (replaceptr + strlen(searchstr) + extralength - buffer) % SCREEN_WIDTH;
                
                memmove(buffer + from + (line * SCREEN_WIDTH), buffer + to + (line * SCREEN_WIDTH), bytes);
                for (i = 0; i <= strlen(replacestr) && replacestr[i] != '\0'; i++) {
                    *(replaceptr + i) = replacestr[i];
                }
                lineoffset += to;
            } else {
                break;
            }
        }
    } else {
        printf("?\n");
        strcpy(error, "line out of range");
    }
}

void search_replace_range(int start, int end, char searchstr[SCREEN_WIDTH], char replacestr[SCREEN_WIDTH]) {
    int i = 0;

    if (start >= 0 && start < lines && start >= 0 && start < lines) {
        for (i = start; i <= end; i++) {
            search_replace(i, searchstr, replacestr);
        }
    } else {
        printf("?\n");
        strcpy(error, "lines out of range");
    }
}

void select_all() {
    if (lines > 0) {
        isRange = 1;
        range.start = 0;
        range.end = lines - 1;
    } else {
        line = 0;
    }
}

void set_surround() {
    isRange = 1;
    if (lines > SURROUND * 2) {
        range.start = line - SURROUND;
        range.end = line + SURROUND;
    } else {
        select_all();
    }
}

void copy_line(int line) {
    char copy_line[SCREEN_WIDTH];
    
    strcpy(copy_line, buffer + (line * SCREEN_WIDTH)); //Copies one line at a time to copyLine
    buffer = update_buffer(copied, copy_lines + 1 * SCREEN_WIDTH * sizeof(char));
    strcpy(copied + (copy_lines * SCREEN_WIDTH), copy_line);
    copy_lines++;
}

void copy_line_range(int start, int end) {
    int i;
    
    for (i = start; i <= end; i++) {
        copy_line(i);
    }
}

void paste(int line) {
    int i;
    
    if (copied == NULL) {
        printf("?\n");
        strcpy(error, "clipboard empty");
    } else {
        for (i = 0; i < copy_lines; i++) {
            insert_line(copied + i * SCREEN_WIDTH, line + 1 + i);
        }
    }
}
    
/* Parses a command and performs an action. Returns 1 when encountered with an error
 * Returns 0 when a quit command is reached
 */
int main(int argc, char *argv[]) {
    char option;
    char cmdopts[] = "p:hv";
    opterr = 0;
    
    char command;
    char command_str[MAX_COMMAND_SIZE];
    int i;
    int linesInputted;
    char lineContents[SCREEN_WIDTH];
    struct number parsedNumber;
    int newLines = 0;
    int c;
    int x;
    int z;
    char prompt = ':';
    char templine[SCREEN_WIDTH]; //Temporary files for use in the transpose command
    char inputLine[SCREEN_WIDTH];
    char *input = NULL;
    char *newInput = NULL;
    int inputSize = 0;
    int fileLines = 0;
    int fileChars = 0;
    char searchstr[SCREEN_WIDTH];
    char replacestr[SCREEN_WIDTH];
    char *replaceptr; //For use by the search & replace command
    char copyLine[SCREEN_WIDTH];
    strcpy(error, "");
    FILE *fp;
    int extralength = 0; //Used by the search and replace function for moving memory on line
    int bytes = 0;
    int to = 0;
    int from = 0;
    int lineoffset = 0; //Stores the line offset when replacing with multiple instances in the same line
    
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
            case 'v':
                printf("See http://github.com/henrik645/octo for more details.\n\n");
                printf("Copyright 2015.\n");
                printf("Licensed by the MIT License.\n");
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
        file_exists = 1;
        strcpy(file_name, argv[optind]);
        fp = fopen(file_name, "r");
        if (fp == NULL) {
            printf(NEW_FILE);
        } else {
            while ((c = fgetc(fp)) != EOF) {
                if (c == '\n') {
                    newLines++;
                }
            }
            rewind(fp);
            lines = newLines;
            
            new_buffer = (char *) realloc(buffer, (lines + 1) * SCREEN_WIDTH);
            if (new_buffer == NULL) {
                printf("Error: out of memory");
                free(buffer);
                exit(2);
            }
            buffer = new_buffer;
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
        file_exists = 0;
        printf(NEW_FILE);
    }
    
    while(1) {
        printf("%c", prompt);
        fgets(command_str, MAX_COMMAND_SIZE, stdin);
        strtok(command_str, "\n"); //Strips the newline away
        for (i = 0; i < strlen(command_str);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
            command = command_str[i];
            parsedNumber = parseInt(command_str, MAX_NUMBER_LEN, i);
            if (parsedNumber.value >= 0) { //Input is number
                i = parsedNumber.size; //parsedNumber.size was already initialized to i beforehand
                if (command_str[i] == ',') {
                    i++; //Removes the ','
                    struct number endNumber = parseInt(command_str, MAX_NUMBER_LEN, i);
                    i = endNumber.size; //endNumber.size was already initialized to i beforehand
                    if (endNumber.value >= 0) {
                        if (parsedNumber.value >= 0 && parsedNumber.value <= lines && endNumber.value >= 0 && endNumber.value <= lines) {
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
                        quit_program();
                        break;
                    case 'n':
                        if (isRange == 1) {
                            print_numbered_lines(range.start, range.end);
                        } else {
                            print_numbered_line(line);
                        }
                        break;
                    case 'e':
                        print_current_line(line);
                        break;
                    case 'c':
                        change_line(line);
                        break;
                    case 'p':
                        if (isRange) {
                            print_lines(range.start, range.end);
                        } else {
                            print_line(line);
                        }
                        break;
                    case 'i':
                        insert_lines(line);
                        break;
                    case 'd':
                        if (isRange == 1) {
                            delete_lines(range.start, range.end);
                        } else {
                            delete_line(line);
                        }
                        break;
                    case 'a':
                        if (lines == 0) {
                            insert_lines(line);
                        } else {
                            insert_lines(line + 1);
                        }
                        break;
                    case 'w':
                        if (write_file() != 0) {
                            strcpy(command_str, ""); //Prevents octo from executing any quit commands as an error occured and the file isn't saved.
                        }
                        break;
                    case 'W':
                        file_exists = 0;
                        if (write_file() != 0) {
                            strcpy(command_str, "");
                        }
                        break;
                    case 'o':
                        if (unsaved == 0) {
                            open_file();
                        } else {
                            printf("!\n");
                            strcpy(error, "unsaved changes");
                        }
                        break;
                    case 't':
                        transpose_next(line);
                        break;
                    case 'T':
                        transpose_previous(line);
                        break;
                    case 'h':
                        print_help();
                        break;
                    case 'f':
                        printf("Search: ");
                        fgets(searchstr, SCREEN_WIDTH, stdin);
                        strtok(searchstr, "\n"); //Removes trailing newline
                        if (isRange == 1) {
                            find_in_range(range.start, range.end, searchstr);
                        } else {
                            if (find_in_line(line, searchstr)) {
                                printf("%d\t%s\n", line + 1, buffer + (line * SCREEN_WIDTH));
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
                            search_replace_range(range.start, range.end, searchstr, replacestr);
                        } else {
                            search_replace(line, searchstr, replacestr);
                        }
                        unsaved = 1;
                        break;
                    case '@':
                        select_all();
                        break;
                    case '!':
                        unsaved = 0;
                        break;
                    case '&':
                        set_surround();
                        break;
                    case 'z':
                        free(copied);
                        copy_lines = 0;
                        copied = NULL;
                        if (isRange == 1) {
                            copy_line_range(range.start, range.end);
                        } else {
                            copy_line(line);
                        }
                        break;
                    case 'x':
                        free(copied);
                        copy_lines = 0;
                        copied = NULL;
                        if (isRange == 1) {
                            copy_line_range(range.start, range.end);
                            delete_lines(range.start, range.end);
                        } else {
                            copy_line(line);
                            delete_line(line);
                        }
                        break;
                    case 'v':
                        paste(line);
                        break;
                    case '\t':
                        break;
                    case ' ':
                        break;
                    case '\n':
                        break;
                    default:
                        printf("?\n");
                        strcpy(command_str, ""); //Empties command_str, accepting no more commands after an error
                        strcpy(error, "unknown command");
                        break;
                }
                i++;
            }
        }
    }
    return 1;
}
