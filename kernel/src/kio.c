#ifndef KIO_H
#define KIO_H
#include <stdarg.h>
#include "./kstring.h"
#include "stdint.h"
#include <stdbool.h>
#include "./io.h"
#include "font.h"

#define BUF_MAX 256
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

#define COM1_PORT 0x3f8
#define BOCHS_PORT 0xe9

void output_to_console(char * str, int size) {
    int i = 0;
    while (str[i] != '\0' && i < size) {
        outb(COM1_PORT, (s8)str[i]);
        outb(BOCHS_PORT, (s8)str[i]);
        i++;
    }
}

static char buffer[BUF_MAX] = {0};

__attribute__((format(printf, 1, 2)))
void ksp(char * f_str, ...) {
    va_list args;
    va_start(args, f_str);

    int size = kvsprintf(buffer, f_str, args);

    output_to_console(buffer, size);
    framebuffer_log_write(buffer, size);
    va_end(args);
}
#endif
