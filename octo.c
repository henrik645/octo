#include <stdio.h>
#include <stdlib.h>

/* Declares a struct number with a value and the number of chars it took up in string form
 * for return from parseInt function.
 */
struct number {
    int value;
    int size;
};

struct number parseInt(char input[], int inputLength);

int main(int argc, char *argv[]) {
    // if (argc != 2 && argc != 3) {
        // printf("Usage: %s filename [prompt]\n", argv[0]);
    // } else {
        // FILE *fp = fopen(argv[1], "r");
        // if (fp == 0) {
            // printf("%s: No such file or directory\n", argv[1]);
        // } else {
            // char command;
            // char prompt;
            // if (argc == 3) {
                // prompt = argv[2];
            // } else {
                // prompt = ':';
            // }
            // while (command != 'q') {
                // printf("%c", prompt); //Prints prompt
                // command = getchar();
                // getchar(); //Flushes the dangling newline
                // printf("%c\n", command);
            // }
        // }
    // }
    // return 0;
    char input[] = "2354";
    printf("Input: %s\n", input);
    struct number integer = parseInt(input, 4);
    printf("Output: %d with size %d\n", integer.value, integer.size);
    return 0;
}

/* Returns -1 if no integer was found and the integer if it was found (only positive values) 
 * It uses the fact that in ASCII, numbers from 0 to 9 comes after each other.
 */
struct number parseInt(char input[], int inputLength) {
    int cursor = 0;
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