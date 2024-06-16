// A very basic string lib with static mem
// no mallocs here.
// Therefore quite portable
// Created on: 13.07.23 22:25
//

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include "./stdint.h"

//TODO: make this a size_t
int strlen(const char* s);
int strcmp(const char * s1, char * s2);
int kvsprintf(char* buffer, char * f_str, va_list args);
