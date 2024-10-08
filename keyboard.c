#include "io.h"
#include "types.h"
#include <keyboard.h>

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
                   '9',  '0',  '\'', '\0', '\0', '\0', 'q',  'w',  'e',  'r',
                   't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
                   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  '\0',
                   '\0', '\0', '\0', '\0', 'z',  'x',  'c',  'v',  'b',  'n',
                   'm',  ',',  '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                   '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

void keyboard_routine() {
    Byte event = inb(0x60);
    // make/break
    // make: key pressed
    // break: key released
    Byte make = !(event >> 7); 
    Byte code = event & 0x7f;
    if (make) {
        char c = char_map[code];
        if (c == '\0') {
            c = 'C';
        }
        printc_xy(79, 24, c);
    }
}

void print_code(Byte c) {
    while (c > 0) {
        printc('0' + (c % 10));
        c /= 10;
    }
    printc(' ');
}