#include "stdint.h"

#include "vmm.h"
#include "task.h"
#include "kio.h"

static struct Task tasks[MAX_TASKS];
static int64_t task_idx_bmp_ptr = 0;

static int64_t current_task_id = 0;
//static int64_t first_time = 0;

void task_init(void (*entry_fn)(void))
{
    struct Task new_task = {0};
    new_task.r.rip = (uint64_t) entry_fn;
    new_task.r.rsp = (uint64_t) vmalloc(1024 * 1024 * 4);

    tasks[task_idx_bmp_ptr++] = new_task;
}

void task_schedule_next(struct regs* r)
{
    if (task_idx_bmp_ptr <= 0) {
        return;
    }

    int64_t next_task_id = (current_task_id + 1) % task_idx_bmp_ptr;
    tasks[current_task_id].r = *r;
    *r = tasks[next_task_id].r;

    current_task_id = next_task_id;
    ksp("Current task id %ld\n", current_task_id);
}
