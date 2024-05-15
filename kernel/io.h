// Reference: https://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
#pragma once

#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void io_wait(void);
