#include "task.h"
#include "pmm.h"

#define MAX_REGION_LIST_TASK 1024

#define STACK_BEGIN_ADDRESS 0x7ff000000000
#define STACK_SIZE 0x100000 // 1Mib;

Task task_init(PmmAllocator* pmm_allocator, u64 current_page_table_address, Elf64 program_elf, u64 argc, char** argv) {
    u64 page_table_address = page_table_create();
    RegionList region_list = regionlist_create(pmm_allocator, MAX_REGION_LIST_TASK);

    { // parsing kernel elf
        void* kernel_address = kfile_request.response->kernel_file->address;
        u64 kernel_size = (u64)kfile_request.response->kernel_file->size;
        Elf64 kernel_elf = elf_parse(kernel_address, kernel_size);
        ASSERT(kernel_elf.p_headers_len > 0);
        ASSERT(kernel_elf.s_headers_len > 0);

        for (u64 i = 0; i < kernel_elf.p_headers_len; i++) {
            Elf64_Phdr pheader = kernel_elf.p_headers[i];
            if (pheader.p_type == PT_LOAD) {
                pheader.p_vaddr = align_down(pheader.p_vaddr);
                pheader.p_memsz = align_up(pheader.p_memsz);

                ASSERT(pheader.p_vaddr % 0x1000 == 0);

                u64 physical_frame = vmm_physical_frame(current_page_table_address, pheader.p_vaddr);

                // reserve the virtual address;
                Region region = region_create(pheader.p_vaddr, pheader.p_memsz);

                u64 flags = FRAME_PRESENT;// | FRAME_USER;

                // if (!(pheader.p_flags & PF_X)) flags |= FRAME_NOEXEC;
                if (pheader.p_flags & PF_W) flags |= FRAME_WRITABLE;

                regionlist_append(&region_list, region);
                region_map(region, page_table_address, physical_frame, flags);
            }
        }
    }


    u64 max_v_address = 0;

    for (u64 i = 0; i < program_elf.p_headers_len; i++) {
        Elf64_Phdr pheader = program_elf.p_headers[i];
        if (pheader.p_type == PT_LOAD) {
            pheader.p_vaddr = align_down(pheader.p_vaddr);
            pheader.p_memsz = align_up(pheader.p_memsz);
            pheader.p_offset = align_down(pheader.p_offset);

            ASSERT(pheader.p_vaddr % 0x1000 == 0);
            ASSERT(pheader.p_offset % 0x1000 == 0);

            // reserve the virtual address;
            Region region = region_create(pheader.p_vaddr, pheader.p_memsz);

            u64 flags = FRAME_PRESENT | FRAME_USER;

            // if (pheader.p_flags & PF_R) region.is_writable = false;
            if (pheader.p_flags & PF_W) flags |= FRAME_WRITABLE; // region.is_writable = true;

            regionlist_append(&region_list, region);
            region_map(region, page_table_address, to_lower_half(pheader.p_offset + (u64)program_elf.elf_module_start), flags);

            if (pheader.p_vaddr + pheader.p_memsz > max_v_address) max_v_address = pheader.p_vaddr + pheader.p_memsz;
        }
    }

    // IMPORTANT: map the kernel on higher half.
    // u64 kernel_text_size = align_up((u64)&_KERNEL_END - (u64)&_KERNEL_START);
    // Region kernel_region = region_create(kernel_address_request.response->virtual_base, kernel_text_size);
    // regionlist_append(&region_list, kernel_region);
    // region_map(kernel_region, page_table_address, kernel_address_request.response->physical_base, FRAME_PRESENT | FRAME_USER);

    // IMPORTANT: create a stack for the executable
    void* stack_frame = pmm_alloc_frame(pmm_allocator, STACK_SIZE >> 12);
    ASSERT(stack_frame);

    Region stack_region = region_create(STACK_BEGIN_ADDRESS, STACK_SIZE);
    regionlist_append(&region_list, stack_region);
    region_map(stack_region, page_table_address, (u64)stack_frame, FRAME_PRESENT | FRAME_WRITABLE | FRAME_USER);

    u64 stack_address = STACK_BEGIN_ADDRESS + STACK_SIZE;
    u64 entry_address = program_elf.header.e_entry;

    // setup the space for argv..............
    /*
    u64 argv_size = 0;
    for (u64 i = 0; i < argc; i++) {
        argv_size += strlen(argv);
    }
    u64 argv_size_pages = align_up(argv_size);
    Region argv_region = region_create(max_v_address, argv_size_pages);
    regionlist_append(&region_list, argv_region);
    void* argv_frame = pmm_alloc_frame(pmm_allocator, STACK_SIZE >> 12);
    region_map(argv_region, page_table_address, (u64)argv_frame, FRAME_PRESENT | FRAME_USER);

    // we need to fill in data into this space, we will temporary map into this space using the current page table adress, fill it and unmap again.
    region_map(argv_region, current_page_table_address, (u64)argv_frame, FRAME_PRESENT | FRAME_WRITABLE);
    u64 current_address = argv_region.start;
    for (int i = 0; i < argc; i++) {
        u64 str_len = strlen(argv[i]);
        memcpy(current_address, argv[i], str_len);
        current_address += str_len;
    }
    region_unmap(argv_region, current_page_table_address);
    */

    Task task = {
        .stack_address = stack_address,
        .page_table_address = page_table_address,
        .entry_address = entry_address,
        .argc = argc,
        .argv = argv,
    };

    return task;
}

void task_set_page_table_and_jump(Task task) {
    u64 page_table_frame = to_lower_half(task.page_table_address);
    __asm__ volatile(
        "\n mov %0, %%cr3"                  // load the page table
        "\n mov %1, %%rsp"                  // change the stack pointer

        "\n pushq %2"
        "\n pushq %3"
                                            //
        // now are setting up the iretq for usermode executable
        "\n mov %%rsp, %%rax"               // save the current stack ptr
        "\n pushq $0x40 | 3"                // stack segment (ss)
        "\n pushq %%rax"                    // rsp (this is the stack address that we saved earlier)
        "\n pushq $0x202"                   // rflags
        "\n pushq $0x38 | 3"                // code segment (cs)
        "\n pushq %4"                       // jump destination
        "\n iretq"
        :
        : "r"(page_table_frame), "r"(task.stack_address), "r"(task.argv), "r"(task.argc), "r"(task.entry_address)
        : "rdi", "rax"
        );
}
