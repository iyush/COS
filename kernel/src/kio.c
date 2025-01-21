#ifndef KIO_H
#define KIO_H
#include <stdarg.h>
#include "./kstring.h"
#include "stdint.h"
#include <stdbool.h>
#include "./io.h"

#define BUF_MAX 256
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

#define COM1_PORT 0x3f8
#define BOCHS_PORT 0xe9

void output_to_console(char * str, int size) {
    int i = 0;
    while (str[i] != '\0' && i < size) {
        outb(COM1_PORT, (uint8_t)str[i]);
        outb(BOCHS_PORT, (uint8_t)str[i]);
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
    va_end(args);
}

static bool is_vga_init = false;
static uint8_t vga_pos_x = 0;
static uint8_t vga_pos_y = 0;

enum VGAColor {
    VGABlack        = 0,
    VGABlue         = 1,
    VGAGreen        = 2,
    VGACyan         = 3,
    VGARed          = 4,
    VGAMagenta      = 5,
    VGABrown        = 6,
    VGALightGray    = 7,
    VGADarkGray     = 8,
    VGALightBlue    = 9,
    VGALightGreen   = 10,
    VGALightCyan    = 11,
    VGALightRed     = 12,
    VGALightMagenta = 13,
    VGAYellow       = 14,
    VGAWhite        = 15,
};

static void output_to_vga(char ch, int pos_x, int pos_y, enum VGAColor foreground, enum VGAColor background) {
    uint8_t attrib  = (foreground & 0x0f) | (background << 4);
    uint16_t * video = (uint16_t *)0xb8000 + VGA_WIDTH * pos_y + pos_x;
    *video = (uint16_t)ch | (attrib << 8);
}


static void output_to_vga_full(uint16_t vga_val, int pos_x, int pos_y) {
    uint16_t * video = (uint16_t *)0xb8000 + VGA_WIDTH * pos_y + pos_x;
    *video = vga_val;
}

static uint16_t * get_vga_val(int row_index, int col_index) {
    uint16_t * mem = (uint16_t*)0xb8000 + VGA_WIDTH * row_index + col_index;
    return mem;
}

static void scroll_up_by_1() {
    // move stuff up by 1
    // TODO: better to do this with memcopy when that is implemented
    for (int row = 0; row < VGA_HEIGHT; row++) {
       for (int col = 0; col < VGA_WIDTH; col++) {
            output_to_vga_full(*get_vga_val(row + 1, col), col, row);
       }
    }

    // clear the last row
    for (int col = 0; col < VGA_WIDTH; col++) {
        output_to_vga(' ', col, VGA_HEIGHT - 1, VGABlack, VGABlack);
    }
}

void vga_clear() {
    for (int row = 0; row < VGA_HEIGHT; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            output_to_vga(' ', col, row, VGABlack, VGABlack);

}

__attribute__((format(printf, 1, 2)))
void kprint(char * fmt, ...) {
    if (!is_vga_init) {
         vga_clear();
         is_vga_init = true;
    }

    va_list args;
    va_start(args, fmt);

    // TODO: could overflow! do dynamic mem alloc here?
    char buffer[BUF_MAX];
    
    int size = kvsprintf(buffer, fmt, args);

    int i = 0;
    while (i < size && buffer[i] != '\0') {
        if (buffer[i] == '\n') {
            vga_pos_y += 1;
            vga_pos_x = 0;
            i++;
            continue;
        }

        if (vga_pos_y >= VGA_HEIGHT) {
            scroll_up_by_1();
            vga_pos_y = VGA_HEIGHT - 1;
        }

        if (vga_pos_x >= VGA_WIDTH) {
            vga_pos_x = 0;
            vga_pos_y += 1;
        }

        output_to_vga(buffer[i], vga_pos_x, vga_pos_y, VGAWhite, VGABlack);
        vga_pos_x += 1;
        i++;
    }

    va_end(args);
}

#endif
