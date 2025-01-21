#pragma once

void vga_clear();

void output_to_console(char * str, int size);
__attribute__((format(printf, 1, 2))) void ksp(char * f_str, ...);
__attribute__((format(printf, 1, 2))) void kprint(char * fmt, ...); 
