
typedef struct TaskList {
    Task* buf;
    u64 capacity;
    u64 len;
} TaskList;



static u64 running_task_id = {0};
static TaskList tasks = {0};

static u64 task_id_counter = 0; // only increments!

#define MAX_QUEUED_TASKS 1024 
static Task tasks_buf[MAX_QUEUED_TASKS] = {0};

void task_list_push(TaskList* list, Task val) {
    if (list->len + 1 > list->capacity) {
        TODO("We do not have enough slot for tasks left!");
    }

    list->buf[list->len] = val;
    list->len++;
}

Task* task_get(TaskList list, u64 idx) {
    if ((idx >= list.len)) {
        UNREACHABLE();
    }

    return &list.buf[idx];
}


void scheduler_init() {
    tasks.buf = tasks_buf;
    tasks.capacity = MAX_QUEUED_TASKS;
}

void scheduler_queue_task(Task task) {
    task.state = TASK_QUEUED;
    task.id = ++task_id_counter;
    task_list_push(&tasks, task);
}

void scheduler_idle_loop() {
    while (true) {
        for (u64 i = 0; i < tasks.len; i++) {
            Task* task = task_get(tasks, i);
            if (task->state == TASK_QUEUED) {
                running_task_id = task->id;
                task->state = TASK_RUNNING;
                ksp("We are now entering the executable\n");
                task_set_page_table_and_jump(*task);
            }
        }
        asm __volatile("hlt");
    }
}


void scheduler_cleanup_task() {
    Task* current_task;
    for (u64 i = 0; i < tasks.len; i++) {
        Task* task = task_get(tasks, i);
        if (task->id == running_task_id) {
            current_task = task;
        }
    }
    ASSERT(current_task);

    current_task->state = TASK_FINISHED;
    ksp("TODO: we need to cleanup task(%ld) as the task has ended!", current_task->id);
    scheduler_idle_loop();
}
