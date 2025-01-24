#ifndef __ASA_TASK
#define __ASA_TASK

typedef struct Task {
    u64 page_table_address;
    u64 stack_address;
    u64 entry_address;
    u64 argc;
    char** argv;
   
} Task;

#endif
