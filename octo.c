#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
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
int is_range = 0;
struct range range;
char *copied = NULL; //Pointer to memory where copied sections of text are stored
int copy_lines = 0; //How many lines are stored there
int e_flag = 0;

/* Declares a struct number with a value and the number of chars it took up in string form
 * for return from parse_int function.
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

/* Declares a struct get_chars with a new begin value and result pointer from return from
   get_chars_until.
 */
struct get_chars {
    int new_begin_at;
    char *result;
};

struct number parse_int(char input[], int inputLength, int inputOffset);

/* Returns -1 if no integer was found and the integer if it was found (only positive values) 
 * It utilises the fact that in ASCII, numbers from 0 to 9 comes after each other.
 */
struct number parse_int(char input[], int inputLength, int inputOffset) {
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

void print_usage(char *program_name) {
    printf("Usage: %s [options] [file name]\n\n", program_name);
    printf("Options:\n");
    printf(" -h: Displays help\n");
    printf(" -p: Sets prompt\n");
    printf(" -v: Displays version\n");
    printf(" -e: Sets command string\n");
}

void print_version() {
    printf("octo v%s\n", VERSION);
}

void *update_buffer(void *buf, size_t size) {
    if (size != 0) {
        void *new_buf = realloc(buf, size);
        if (new_buf == NULL) {
            fprintf(stderr, "Error: not enough memory");
            exit(2);
        }
        buf = new_buf;
    }
    return buf;
}

int count_in_str(const char *substr, const char *str) {
    int count = 0;
    int nomatch;
    regex_t exp;
    regmatch_t matches[1];

    if (regcomp(&exp, substr, 0) != 0) {
        return -1;
    }

    while (1) {
        nomatch = regexec(&exp, str, 1, matches, 0);
        if (nomatch) {
            break;
        }
        
        str += matches[0].rm_eo;
        count++;
    }
    return count;
}

struct get_chars get_chars_until(char *string, int begin_at, char separator, int characters) {
    int result_index = 0;
    char *result_str = malloc((characters + 1) * sizeof(char)); //Needs extra space for the final null byte
    struct get_chars result;
    
    if (begin_at > strlen(string)) {
        result.result = NULL;
        return result;
    }
    while (string[begin_at] != separator && strlen(string) - begin_at > 0) {
        result_str[result_index++] = string[begin_at++];
    }

    result_str[result_index] = '\0';

    result.new_begin_at = begin_at;
    result.result = result_str;
    return result;
}

void print_error(char *msg) {
    if (e_flag) {
        printf("error: %s\n", msg);
    } else {
        printf("?\n");
        strcpy(error, msg);
    }
}

void print_warning(char *msg) {
    if (e_flag) {
        printf("warning: %s\n", msg);
    } else {
        printf("!\n");
        strcpy(error, msg);
    }
}

void print_numbered_line(int line) {
    char lineContents[SCREEN_WIDTH];
    if (line + 1 > lines || line + 1 < 1) {
        print_error("line entered is outside limits");
    } else {
        strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
        printf("%d\t%s\n", line + 1, lineContents);
    }
}

void print_numbered_lines(int start, int end) {
    int i;
    if (start + 1 > lines || end + 1 > lines || start < 0 || end < 0) {
        print_error("range outside limits");
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
        print_warning("unsaved changes");
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
            fprintf(stderr, "print_error: out of memory");
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
        print_error("line entered is outside limits");
    } else {
        strcpy(lineContents, buffer + (line * SCREEN_WIDTH));
        printf("%s\n", lineContents);
    }
}

void print_lines(int start, int end) {
    int i;
    
    if (start + 1 > lines || end + 1 > lines || start < 0 || end < 0) {
        print_error("range outside limits");
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
        fprintf(stderr, "print_error deleting line");
        free(buffer);
        exit(2);
    }
    buffer = new_buffer;
    
    unsaved = 1;
}

void insert_lines(int current_line) {
    int lines_inputted = 0;
    char input_line[SCREEN_WIDTH];
    char c;
    
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
    if (line + 1 >= 1 && line + 1 <= lines) {
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
    buffer = update_buffer(buffer, lines * SCREEN_WIDTH * sizeof(char));
}

void delete_lines(int start, int end) {
    int i;
    
    if (start + 1 <= lines && end + 1 <= lines && start + 1 >= 1 && end + 1 >= 1) {
        for (i = start; i <= end; i++) {
            delete_line(start); //When we are deleting a line, the rest of the lines will be shifted down. Thus, each new line will appear at 'start'
        }
    } 
}

int write_file_name(char file_name[SCREEN_WIDTH]) { //Returns -1 on print_error, the number of characters written otherwise
    int i = 0;
    int x = 0;
    int file_chars = 0;
    char c;
    
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        return -1;
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
    unsaved = 0;
    file_exists = 1;
    
    return file_chars;
}

int write_file() { //Returns -1 on print_error, otherwise the amount of characters read
    int file_chars;
    
    if (file_exists) {
        return write_file_name(file_name);
    }

    printf("File: ");
    fgets(file_name, SCREEN_WIDTH, stdin);
    strtok(file_name, "\n"); //Removes the trailing newline
    
    file_chars = write_file_name(file_name);
    if (file_chars == -1) {
        file_exists = 0;
        fprintf(stderr, "file could not be written");
        return -1;
    }
    
    return file_chars;
}

int open_file(FILE *fp) {
    int file_lines = 0;
    int file_chars = 0;
    int c; // int needed for proper handling of EOF
    int y = 0;
    int x = 0;
    int longest_line = 0;
    
    if (fp == NULL) {
        print_error("file not found");
        return -1;
    }
    
    file_exists = 1;
    while ((c = fgetc(fp)) != EOF) { //Counts the file size for sizing of the buffer
        if (c == '\n') {
            file_lines++;
            if (file_chars > longest_line) {
                longest_line = file_chars;
            }
            file_chars = 0;
        } else {
            file_chars++;
        }
    }

    file_chars = 0;
    
    if (longest_line > SCREEN_WIDTH) {
        file_exists = 0;
        print_warning("file is wider than screen width");
        return -1;
    } else {
        lines = file_lines;
        free(buffer);
        buffer = NULL; //Allows the buffer to be safely reallocated
        buffer = update_buffer(buffer, (lines + 1) * SCREEN_WIDTH * sizeof(char)); //Reallocates the buffer to the desired size
        rewind(fp); //Rewinds the file for reading actual file contents
        
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                *(buffer + (y * SCREEN_WIDTH) + x) = '\0';
                x = 0;
                y++;
            } else {
                *(buffer + (y * SCREEN_WIDTH) + x) = c;
                x++;
                file_chars++;
            }
        }
        fclose(fp);
        return file_chars;
    }
}

void open_file_prompt() {
    char file_name[SCREEN_WIDTH];
    FILE *fp;
    int file_chars;
    
    printf("File: ");
    fgets(file_name, SCREEN_WIDTH, stdin);
    strtok(file_name, "\n"); //Removes the trailing newline
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf(NEW_FILE);
        file_exists = 1;
        free(buffer);
        buffer = NULL;
        lines = 0;
        line = 0;
        is_range = 0;
    } else {
        file_chars = open_file(fp);
        if (file_chars >= 0) {
            printf("%d\n", file_chars);
        }
    }
}

void transpose_next(int line) {
    char templine[SCREEN_WIDTH];
    
    if (line + 2 <= lines && line + 1 >= 1) {
        strcpy(templine, buffer + ((line + 1) * SCREEN_WIDTH));
        strcpy(buffer + ((line + 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
        strcpy(buffer + (line * SCREEN_WIDTH), templine);
    } else {
        print_error("can't transpose last line");
    }
    unsaved = 1;
}

void transpose_previous(int line) {
    char templine[SCREEN_WIDTH];
    
    if (line + 1 <= lines && line + 1 >= 2) {
        strcpy(templine, buffer + ((line - 1) * SCREEN_WIDTH));
        strcpy(buffer + ((line - 1) * SCREEN_WIDTH), buffer + (line * SCREEN_WIDTH));
        strcpy(buffer + (line * SCREEN_WIDTH), templine);
    } else {
        print_error("can't transpose first line");
    }
}

void print_help() {
    if (error[0] == '\0') {
        print_error("no error found");
    } else {
        printf("%s\n", error);
    }
}

void find_in_line(int line, char searchstr[SCREEN_WIDTH]) {
    regex_t exp;

    if (line >= 0 && line < lines) {
        if (regcomp(&exp, searchstr, 0) != 0) {
            print_error("malformed regular expression");
        } else {
            if (regexec(&exp, buffer + line * SCREEN_WIDTH, 0, NULL, 0) == 0) { //match was found
                printf("%d\t%s\n", line + 1, buffer + (line * SCREEN_WIDTH));
            }
            regfree(&exp);
        }
    } else {
        print_error("line out of range");
    }
}

void find_in_range(int start, int end, char searchstr[SCREEN_WIDTH]) {
    int i = 0;
    
    if (start + 1 >= 1 && start + 1 <= lines && end + 1 >= 1 && start + 1 <= lines) {
        for (i = start; i <= end; i++) {
            find_in_line(i, searchstr);
        }
    }
}

int search_replace(int line, char searchstr[SCREEN_WIDTH], char replacestr[SCREEN_WIDTH]) {
    int i = 0;
    int extralength = 0;
    int bytes;
    int to;
    int from;
    int n_matches = 1;
    int nomatch;
    char *previous_match = buffer + line * SCREEN_WIDTH;
    regex_t exp;
    regmatch_t matches[n_matches];
    regmatch_t match;
    
    if (line >= 0 && line + 1 <= lines) {
        if (regcomp(&exp, searchstr, 0) != 0) {
            print_error("malformed regular expression");
        } else {
            while (1) {
                nomatch = regexec(&exp, previous_match, n_matches, matches, 0);
                if (nomatch) {
                    break;
                }
                match = matches[0];
                if (count_in_str(searchstr, buffer + line * SCREEN_WIDTH) * (strlen(replacestr) - (match.rm_eo - match.rm_so)) + strlen(buffer + line * SCREEN_WIDTH) > SCREEN_WIDTH - 1) { // One for the NULL string character
                    print_error("line is not wide enough for replacing of all instances");
                    return 0;
                }

                extralength = strlen(replacestr) - (match.rm_eo - match.rm_so);
                
                if (extralength >= 0) {
                    from = match.rm_so;
                } else {
                    from = match.rm_eo;
                }
                to = from + extralength;
                bytes = SCREEN_WIDTH - (match.rm_so + extralength);

                memmove(to + previous_match, from + previous_match, bytes);
                for (i = 0; i < strlen(replacestr) && replacestr[i] != '\0'; i++) {
                    *(match.rm_so + previous_match + i) = replacestr[i];
                }
                previous_match += match.rm_eo + 1; 
            }
            regfree(&exp);
        }
    } else {
        print_error("line out of range");
        return 0;
    }
    return 1;
}

void search_replace_range(int start, int end, char searchstr[SCREEN_WIDTH], char replacestr[SCREEN_WIDTH]) {
    int i = 0;

    if (start >= 0 && start < lines && start >= 0 && start < lines) {
        for (i = start; i <= end; i++) {
            if (search_replace(i, searchstr, replacestr) == 0) {
                break;
            }
        }
    } else {
        print_error("lines out of range");
    }
}

void select_all() {
    if (lines > 0) {
        is_range = 1;
        range.start = 0;
        range.end = lines - 1;
    } else {
        line = 0;
    }
}

void set_surround() {
    is_range = 1;
    if (line + 1 >= 1 && line + 1 <= lines) {
        if (lines > SURROUND * 2) {
            if (line < SURROUND) {
                range.start = 0;
                range.end = SURROUND * 2;
            } else if ((lines - 1) - line < SURROUND) {
                range.start = lines - (SURROUND * 2) - 1;
                range.end = lines - 1;
            } else {
                range.start = line - SURROUND;
                range.end = line + SURROUND;
            }
        } else {
            select_all();
        }
    } else {
        print_error("line out of range");
    }
}

void set_last_line() {
    is_range = 0;
    line = lines;
}

void copy_line(int line) {
    char copy_line[SCREEN_WIDTH];
    
    if (line + 1 >= 1 && line + 1 <= lines) {
        strcpy(copy_line, buffer + (line * SCREEN_WIDTH)); //Copies one line at a time to copyLine
        copied = update_buffer(copied, (copy_lines + 1) * SCREEN_WIDTH * sizeof(char));
        strcpy(copied + (copy_lines * SCREEN_WIDTH), copy_line);
        copy_lines++;
    } else {
        print_error("line out of range");
    }
}

void copy_line_range(int start, int end) {
    int i;
    
    if (start + 1 >= 1 && start + 1 <= lines && end + 1 >= 1 && end + 1 <= lines) {
        for (i = start; i <= end; i++) {
            copy_line(i);
        }
    } else {
        print_error("lines out of range");
    }
}

void paste(int line) {
    int i;
    
    if (line + 1 >= 1 && line + 1 <= lines) {
        if (copied == NULL) {
            print_error("clipboard empty");
        } else {
            for (i = 0; i < copy_lines; i++) {
                insert_line(copied + i * SCREEN_WIDTH, line + 1 + i);
            }
        }
    }
}

/* Parses a command and performs an action. Returns 1 when encountered with an error
 * Returns 0 when a quit command is reached
 */

void parse_commands(char *command_str) {
    int i;
    int file_chars;
    char searchstr[SCREEN_WIDTH];
    char replacestr[SCREEN_WIDTH];
    char command;
    struct number parsed_number;
    struct number end_number;
    struct get_chars result;
    
    for (i = 0; i < strlen(command_str);) { //i is not incremented by loop but instead by the code below depending on if the command is a number or not
        command = command_str[i];
        parsed_number = parse_int(command_str, MAX_NUMBER_LEN, i);
        if (parsed_number.value >= 0) { //Input is number
            i = parsed_number.size; //parsed_number.size was already initialized to i beforehand
            if (command_str[i] == ',') {
                i++; //Removes the ','
                end_number = parse_int(command_str, MAX_NUMBER_LEN, i);
                i = end_number.size; //endNumber.size was already initialized to i beforehand
                if (end_number.value >= 0) {
                    if (parsed_number.value >= 0 && parsed_number.value <= lines && end_number.value >= 0 && end_number.value <= lines && parsed_number.value <= end_number.value) {
                        is_range = 1;
                        range.start = parsed_number.value - 1;
                        range.end = end_number.value - 1;
                    } else {
                        print_error("range limits out of range");
                        strcpy(command_str, "");
                    }
                } else {
                    print_error("wrongly formatted range");
                    break;
                }
            } else {
                is_range = 0;
            }
            line = parsed_number.value - 1; //To account for the shifting (see above at initialization)
        } else {
            if (line + 1 > lines || line + 1 < 1) {
                if (lines == 0) {
                    line = lines;
                } else {
                    line = lines - 1;
                }
            }
            switch (command) {
                case 'q':
                    quit_program();
                    break;
                case 'n':
                    if (is_range == 1) {
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
                    if (is_range) {
                        print_lines(range.start, range.end);
                    } else {
                        print_line(line);
                    }
                    break;
                case 'i':
                    insert_lines(line);
                    break;
                case 'd':
                    if (is_range == 1) {
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
                    file_chars = write_file();
                    if (file_chars == -1) {
                        strcpy(command_str, ""); //Prevents octo from executing any quit commands as an error occured and the file isn't saved.
                    } else {
                        printf("%d\n", file_chars);
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
                        open_file_prompt();
                    } else {
                        print_warning("unsaved changes");
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
                    i += 2; //Winds past the 'f/'

                    result = get_chars_until(command_str, i, '/', MAX_COMMAND_SIZE);

                    if (result.result == NULL) {
                        print_error("search string needs to be specified");
                        break;
                    }

                    i = result.new_begin_at;
                    strcpy(searchstr, result.result);

                    i++; //Winds past the final '/'

                    if (is_range == 1) {
                        find_in_range(range.start, range.end, searchstr);
                    } else {
                        find_in_line(line, searchstr);
                    }
                    break;
                case 's':
                    i += 2; //Winds past the 's/'

                    result = get_chars_until(command_str, i, '/', MAX_COMMAND_SIZE);

                    if (result.result == NULL) {
                        print_error("search string needs to be specified");
                        break;
                    }

                    i = result.new_begin_at;
                    strcpy(searchstr, result.result);

                    i++; //Winds past the '/'

                    result = get_chars_until(command_str, i, '/', MAX_COMMAND_SIZE);

                    if (result.result == NULL) {
                        print_error("replace string needs to be specified");
                        break;
                    }

                    i = result.new_begin_at;
                    strcpy(replacestr, result.result);

                    if (is_range == 1) {
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
                case '$':
                    set_last_line();
                    break;
                case 'z':
                    free(copied);
                    copy_lines = 0;
                    copied = NULL;
                    if (is_range == 1) {
                        copy_line_range(range.start, range.end);
                    } else {
                        copy_line(line);
                    }
                    break;
                case 'x':
                    free(copied);
                    copy_lines = 0;
                    copied = NULL;
                    if (is_range == 1) {
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
                case 'r':
                    if (file_exists) {
                        open_file(fopen(file_name, "r"));
                    } else {
                        print_error("no file open");
                    }
                    break;
                case '\t':
                    break;
                case ' ':
                    break;
                case '\n':
                    break;
                default:
                    strcpy(command_str, ""); //Empties command_str, accepting no more commands after an error
                    print_error("unknown command");
                    break;
            }
            i++;
        }
    }
}
    
int main(int argc, char *argv[]) {
    int file_chars = 0;
    char prompt = ':';
    char option;
    char cmdopts[] = "p:hve:";
    char command_str[MAX_COMMAND_SIZE];
    FILE *fp;
    opterr = 0;
    
    strcpy(error, "");

    while ((option = getopt(argc, argv, cmdopts)) != -1) {
        switch (option) {
            case 'p':
                prompt = optarg[0];
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
                break;
            case 'v':
                print_version();
                printf("See http://github.com/henrik645/octo for more details.\n\n");
                printf("Copyright 2015.\n");
                printf("Licensed by the MIT License.\n");
                return 0;
                break;
            case 'e':
                strcpy(command_str, optarg);
                e_flag = 1;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr, "error: 'p' requires an argument.\n");
                    return 1;
                } else if (optopt == 'e') {
                    fprintf(stderr, "error: 'e' requires an argument.\n");
                    return 1;
                } else if (isprint(optopt)) {
                    fprintf(stderr, "error: Unknown option -%c\n", optopt);
                    return 1;
                } else {
                    fprintf(stderr, "error: Unknown option.");
                    return 1;
                }
                break;
            default:
                abort();
        }
    }
    
    if (!e_flag) {
        print_version();
    }
    
    if (argc - optind > 1) { //Too many arguments
        print_usage(argv[0]);
        exit(1);
    } else if (optind + 1 == argc) {
        file_exists = 1;
        strcpy(file_name, argv[optind]);
        fp = fopen(file_name, "r");
        if (fp == NULL) {
            printf(NEW_FILE);
        } else {
            file_chars = open_file(fp);
            if (!e_flag && file_chars >= 0) {
                printf("%d\n", file_chars);
            }
        }
    } else {
        if (e_flag) {
            printf("'e' option requires a file to work on\n");
            return 1;
        }
        file_exists = 0;
        printf(NEW_FILE);
    }
    
    while(1) {
        if (!e_flag) {
            printf("%c", prompt);
            fgets(command_str, MAX_COMMAND_SIZE, stdin);
            strtok(command_str, "\n"); //Strips the newline away
            parse_commands(command_str);
        } else {
            parse_commands(command_str);
            write_file_name(file_name);
            quit_program();
        }
    }
    return 1;
}
