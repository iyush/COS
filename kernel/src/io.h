// Reference: https://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
#pragma once

#include "stdint.h"

void outb(u16 port, u8 val);
u8 inb(u16 port);
void io_wait(void);
