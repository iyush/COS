#ifndef KIO_H
#define KIO_H
#include <stdarg.h>
#include "./kstring.h"
#include <stdbool.h>

#define VGA_HEIGHT 25
#define VGA_WIDTH 80

typedef __INT8_TYPE__  int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef __UINT8_TYPE__  uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

static void output_to_console(char * str, int size) {
  asm volatile(
      "mov %0, %%esi\n"    // Load the address of myString into ESI register
      "mov $0x402, %%dx\n" // Example port number for serial port (COM1)
      "cld\n"              // Set the direction flag to forward
      "mov %1, %%ecx\n"   // Length of the string excluding null character
      "rep outsb\n"        // Output the string
      :
      : "g"(str), "r"(size)// Input constraint: the address of myString
      : "esi", "edx", "ecx" // Clobbered registers
  );
}

void ksp(char * f_str, ...) {
    va_list args;
    va_start(args, f_str);

    char buffer[BUF_MAX];
    
    int size = kvsprintf(buffer, BUF_MAX, f_str, args);

    output_to_console(buffer, size);
    va_end(args);
}


enum VGAColor: uint8_t {
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

void output_to_vga(char ch, int pos_x, int pos_y, enum VGAColor foreground, enum VGAColor background) {

    uint8_t attrib  = (foreground & 0x0f) | (background << 4);

    uint16_t * video = (uint16_t *)0xb8000 + VGA_WIDTH * pos_y + pos_x;
    *video = (uint16_t)ch | (attrib << 8);
}

static bool is_vga_init = false;
static uint8_t vga_pos_x = 0;
static uint8_t vga_pos_y = 0;

static void vga_init() {
    for (int row = 0; row < VGA_HEIGHT; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            output_to_vga(' ', col, row, VGABlack, VGABlack);

    is_vga_init = true;
}


static void scroll_up_by_1() {
    
}

void kprint(char * fmt, ...) {
    if (!is_vga_init) {
        ksp("vga init is called");
        vga_init();
    }

    va_list args;
    va_start(args, fmt);

    char buffer[BUF_MAX];
    
    int size = kvsprintf(buffer, BUF_MAX, fmt, args);

    int i = 0;
    while (i < size && buffer[i] != '\0') {
        if (buffer[i] == '\n') {
            vga_pos_y += 1;
            vga_pos_x = 0;
            i++;
            continue;
        }

        output_to_vga(buffer[i], vga_pos_x, vga_pos_y, VGAWhite, VGABlack);

        vga_pos_x += 1;
        if (vga_pos_x >= VGA_WIDTH) {
            vga_pos_x = 0;
            vga_pos_y += 1;
        }

        if (vga_pos_y >= VGA_HEIGHT) {
            scroll_up_by_1();
        }

        i++;
    }

    va_end(args);
}

#endif
