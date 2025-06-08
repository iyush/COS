#include "keyboard.h"
#include "kio.h"
#include "io.h"
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

void keyboard_handle_interrupt() {
    s8 scancode = inb((u16)KEYBOARD_DATA_PORT);
    if (scancode > 0) {
        char c = scancode_to_ascii[scancode];
        if (c) {
            kprint("%c", c);
        }
    }
    pic_send_end_of_interrupt();
}
