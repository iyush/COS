#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_BUFFER_SIZE 128  // Define the buffer size

typedef struct KeyboardBuffer {
    char buffer[KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
} KeyboardBuffer;

extern KeyboardBuffer g_keyboard_buffer;

void keyboard_handle_interrupt();
char keyboard_getchar();
#endif
