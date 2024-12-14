#pragma once

#include "stdint.h"
#include "idt.h"

#define MAX_TASKS 1024

struct Task {
    int32_t id;
    struct regs r;
};


void task_init(void (*entry_fn)(void));
void task_schedule_next(struct regs* r);
