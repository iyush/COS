#ifndef FONT_H
#define FONT_H

#include "asa_limine.h"
#include <stddef.h>

#define FRAMEBUFFER_LOG_CHAR_WIDTH 8
#define FRAMEBUFFER_LOG_CHAR_HEIGHT 8

#define MAX_FRAMEBUFFER_ROWS 128
#define MAX_FRAMEBUFFER_COLS 256


typedef enum {
    FONT_COLOR_GREEN   = 0x00ff00,
    FONT_COLOR_RED     = 0xff0000,
    FONT_COLOR_BLUE    = 0x0000ff,
    FONT_COLOR_YELLOW  = 0xffff00,
    FONT_COLOR_WHITE   = 0xffffff,
    FONT_COLOR_CYAN    = 0x00ffff,
    FONT_COLOR_MAGENTA = 0xff00ff,
    FONT_COLOR_BLACK   = 0x000000,
    FONT_COLOR_ORANGE  = 0xffa500
} FontColor;

typedef struct {
    struct limine_framebuffer *fb;
    unsigned int *fb_ptr;
    int fb_width;
    int fb_height;
    int char_width;
    int char_height;
    int rows;
    int cols;
    int cursor_row;
    int cursor_col;
    char char_grid[MAX_FRAMEBUFFER_ROWS][MAX_FRAMEBUFFER_COLS+1];
    char *char_grid_ptrs[MAX_FRAMEBUFFER_ROWS];
} FramebufferContext;

void draw_char(unsigned int* fb, int fb_width, int x, int y, char c, unsigned int color);
void draw_string(unsigned int* fb, int fb_width, int x, int y, const char* s, unsigned int color);
void framebuffer_log_init();
void framebuffer_log_write(const char* str, int size);

#endif
