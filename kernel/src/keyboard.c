#include "keyboard.h"
#include "kio.h"
#include "pic.h"
#include "stdint.h"

#define KEYBOARD_DATA_PORT 0x60

static const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    // rest are zeros
};

// Only define the global variable, not the struct
KeyboardBuffer g_keyboard_buffer = { .buffer = {0}, .head = 0, .tail = 0 };

int keyboard_buffer_is_empty() {
    return g_keyboard_buffer.head == g_keyboard_buffer.tail;
}

int keyboard_buffer_is_full() {
    return ((g_keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE) == g_keyboard_buffer.tail;
}

void keyboard_buffer_push(char c) {
    if (!keyboard_buffer_is_full()) {
        g_keyboard_buffer.buffer[g_keyboard_buffer.head] = c;
        g_keyboard_buffer.head = (g_keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
    }
}

char keyboard_getchar() {
    if (keyboard_buffer_is_empty()) return 0;
    char c = g_keyboard_buffer.buffer[g_keyboard_buffer.tail];
    g_keyboard_buffer.tail = (g_keyboard_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

void keyboard_handle_interrupt() {
    s8 scancode = inb((u16)KEYBOARD_DATA_PORT);
    if (scancode > 0) {
        char c = scancode_to_ascii[scancode];
        if (c) {
            keyboard_buffer_push(c);
        }
    }
    pic_send_end_of_interrupt();
}
