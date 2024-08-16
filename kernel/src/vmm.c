#include <pmm.h>
#include <stdint.h>
#include <limine.h>
#include <mem.h>
#include <vmm.h>
#include <assert.h>

extern uint64_t _KERNEL_START;
extern uint64_t _KERNEL_END;

#define RECURSIVE_PTE_INDEX 0x142ul

uint64_t _vmm_cr3()
{
    uint64_t cr3;

    asm __volatile__(
        "mov %%cr3, %0\n"
        : "=r"(cr3)
        :
        :);

    return cr3;
}

void panic(char * msg)
{
    ksp("PANIC!!!!!!\n\t %s\n", msg);
    while(1) {}
}

/*
Plan:

Page Tables are all heirarchical, i.e p4 should have a p3 table, p2, p1 tables.
p4 table should have an entry pointing to itself, __recursive page table__.

If we go down the recursive page table path, we can only modify the page
tables that is currently pointed to cr3. In order to create entries of
the new page table, we have to somehow map the new page table into
an exisiting table, modify or zero out the entries and then unmap
the page table.

For this we create two distinct page tables:
1. ActivePageTable
2. InactivePageTable

We would normally like to reuse the page


ActivePageTable: page table that is currently pointed in the cr3 register
InactivePageTable: page table that is not pointed in the cr3 register.

We need to create an InactivePageTable,

*/

// utilize the fact that there are recursive entries to set the map here.
void vmm_map_page(uint64_t* pml4e_table, void * v_start, void* p_start)
{
    if (((uint64_t)p_start & 0xfff) != 0)
    {
        panic("vmm_map_page: p_start is not a valid 4kb physical page address.");
    }
    if (((uint64_t)v_start & 0xfff) != 0)
    {
        panic("vmm_map_page: v_start is not a valid 4kb virtual page address.");
    }

    uint64_t pml4e_offset = (uint64_t)(((uint64_t)v_start >> 39) & 0x01ff);
    uint64_t pdpte_offset = (uint64_t)(((uint64_t)v_start >> 30) & 0x01ff);
    uint64_t pde_offset   = (uint64_t)(((uint64_t)v_start >> 21) & 0x01ff);
    uint64_t pte_offset   = (uint64_t)(((uint64_t)v_start >> 12) & 0x01ff);

    if (!(pml4e_table[pml4e_offset] & FRAME_PRESENT))  pml4e_table[pml4e_offset] = (uint64_t)(pmm_alloc_frame(1)) | FRAME_PRESENT | FRAME_WRITABLE;
    uint64_t * pdpte_table  = (uint64_t*) ((RECURSIVE_PTE_INDEX << 39) | (RECURSIVE_PTE_INDEX << 30) | (RECURSIVE_PTE_INDEX << 21)   | (pml4e_offset << 12));

    if (!(pdpte_table[pdpte_offset] & FRAME_PRESENT))
    {
        pdpte_table[pdpte_offset] = (uint64_t)(pmm_alloc_frame(1)) | FRAME_PRESENT | FRAME_WRITABLE; 
    }
    uint64_t * pde_table    = (uint64_t *)((RECURSIVE_PTE_INDEX << 39) | (RECURSIVE_PTE_INDEX << 30) | (pml4e_offset << 21)          | (pde_offset << 12));
    
    if (!(pde_table[pde_offset] & FRAME_PRESENT))
    {
        pde_table[pde_offset] = (uint64_t)(pmm_alloc_frame(1)) | FRAME_PRESENT | FRAME_WRITABLE; 
    }
    uint64_t * p_table      = (uint64_t*)((RECURSIVE_PTE_INDEX << 39) | (pml4e_offset << 30)        | (pde_offset << 21)            | (pte_offset << 12));

    p_table[pte_offset] = (uint64_t)p_start | FRAME_PRESENT | FRAME_WRITABLE;
}

void vmm_pg_dmp(uint64_t* page_table) 
{
    for (int table_idx = 0; table_idx < 512; table_idx++)
    {
        if ((page_table[table_idx] & FRAME_PRESENT))
        {
            ksp("%d %lx\n", table_idx, page_table[table_idx]);      
        }
    }
}

uint64_t* vmm_get_p3(uint64_t p3_offset)
{
    return (uint64_t*)(
            0xFFFF000000000000ULL | 
            (RECURSIVE_PTE_INDEX << 39) | 
            (RECURSIVE_PTE_INDEX << 30) | 
            (RECURSIVE_PTE_INDEX << 21) | 
            (p3_offset << 12)
        );
}


uint64_t* vmm_get_p2(uint64_t p3_offset, uint64_t p2_offset)
{
    return (uint64_t*)(
            0xFFFF000000000000ULL | 
            (RECURSIVE_PTE_INDEX << 39) | 
            (RECURSIVE_PTE_INDEX << 30) | 
            (p3_offset << 21) | 
            (p2_offset << 12)
        );
}


uint64_t* vmm_get_p1(uint16_t p3_offset, uint64_t p2_offset, uint64_t p1_offset)
{
    return (uint64_t*)(
            0xFFFF000000000000ULL | 
            ((uint64_t)RECURSIVE_PTE_INDEX << 39) | 
            ((uint64_t)p3_offset << 30) | 
            ((uint64_t)p2_offset << 21) | 
            ((uint64_t)p1_offset << 12)
        );
}

typedef struct 
{
    uint64_t p1_offset;
    uint64_t p2_offset;
    uint64_t p3_offset;

    uint64_t* p1_frame;
    uint64_t* p2_frame;
    uint64_t* p3_frame;

    uint64_t* p2_page;
    uint64_t* p3_page;
} TemporaryPTHeir;

// a page table heirarchy containing p1 p2 and p3 table, with offsets, used for temporarily mapping page tables and editing them.
static TemporaryPTHeir vmm_temporary_page_tables_init(uint64_t* p4_table)
{
    TemporaryPTHeir tp = {};

    tp.p3_frame = pmm_alloc_frame(1);
    tp.p2_frame = pmm_alloc_frame(1);
    tp.p1_frame = pmm_alloc_frame(1);

    p4_table[tp.p3_offset] = ((uint64_t)tp.p3_frame) | FRAME_WRITABLE | FRAME_PRESENT;

    tp.p3_page = vmm_get_p3(tp.p3_offset);
    tp.p3_page[tp.p2_offset] = ((uint64_t)tp.p2_frame) | FRAME_WRITABLE | FRAME_PRESENT;

    tp.p2_page = vmm_get_p2(tp.p3_offset, tp.p2_offset); 
    tp.p2_page[tp.p1_offset] = ((uint64_t)tp.p1_frame) | FRAME_WRITABLE | FRAME_PRESENT;
    
    return tp;
}


static uint64_t* vmm_temporary_page_tables_map(TemporaryPTHeir* temp_pt_heir, void* phy_frame)
{
    uint16_t frame_offset = 123;
    TemporaryPTHeir* tp = temp_pt_heir;

    uint64_t* p1_page = vmm_get_p1(tp->p3_offset, tp->p2_offset, tp->p1_offset); 
    if (p1_page[frame_offset] & FRAME_PRESENT) 
    {
        // probably we do not care.
        //panic("A frame should never be present here, did you forget to unmap it previously?");
    }

    p1_page[frame_offset] = ((uint64_t)phy_frame) | FRAME_WRITABLE | FRAME_PRESENT;

    uint64_t* new_p4_page = (uint64_t*)(
            ((uint64_t)tp->p3_offset << 39) | 
            ((uint64_t)tp->p2_offset << 30) | 
            ((uint64_t)tp->p1_offset << 21) | 
            ((uint64_t)frame_offset  << 12)
        );

    // flush tlb cache
    uint64_t cr3 = _vmm_cr3();
    __asm__ volatile("mov %0, %%cr3" : : "r" (cr3) : "memory");

    return new_p4_page;
}

static void vmm_temporary_page_tables_deinit(TemporaryPTHeir tp)
{
    // unmap the page table
    tp.p2_page[tp.p1_offset] = 0;
    tp.p3_page[tp.p2_offset] = 0;
    VMM_P4[tp.p3_offset] = 0;

    pmm_dealloc_frame(tp.p1_frame, 1);
    pmm_dealloc_frame(tp.p2_frame, 1);
    pmm_dealloc_frame(tp.p3_frame, 1);

    // flush tlb cache
    uint64_t cr3 = _vmm_cr3();
    __asm__ volatile("mov %0, %%cr3" : : "r" (cr3) : "memory");
}

void vmm_init(struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kadd_request, struct limine_kernel_file_request kfile_request)
{
    // recursive map the current page table, p4 to itself
    uint64_t current_p4_frame =  _vmm_cr3();
    uint64_t * current_p4_page  = (uint64_t*)(current_p4_frame + hhdm_request.response->offset);
    current_p4_page[RECURSIVE_PTE_INDEX] = current_p4_frame | FRAME_PRESENT | FRAME_WRITABLE;


    vmm_pg_dmp(current_p4_page);

    // As an exercise: 
    // allocate a frame for a new page table
    // temporarily map the new page table to the current page table
    // zero out all the entries
    // unmap the new page table
    // temporarily remap the new page table
    // fill in the identical entries as the currently active page table, namely:
    //       1. page table entry for kernel code.
    //       2. page table entry for the higher half direct memory (logical address).
    //       3. page table entry for kernel malloc region          (virtual address).
    // unmap the page table.
    // switch to the new page table by moving cr3 register.

    TemporaryPTHeir temp_pt_heir = vmm_temporary_page_tables_init(current_p4_page);
    
    for (int j = 0; j < 10; j++)
    {
        // allocate a frame for a new page table
        uint64_t *new_p4_frame = pmm_alloc_frame(1);

        // temporarily map the page table to the current page table

        uint64_t * page_addr = vmm_temporary_page_tables_map(&temp_pt_heir, new_p4_frame);
        // ksp("%lx\n", (uint64_t)new_p4_frame);
        vmm_pg_dmp(page_addr);

    	// zero the entire page
        memset((uint8_t *)page_addr, 0x33, 4096);

        //vmm_pg_dmp(page_addr);

        // unmap the temporary page tables
    }
    vmm_temporary_page_tables_deinit(temp_pt_heir);

    struct limine_file* kfile = kfile_request.response->kernel_file;
    ksp("kernel file size %ld \n", kfile->size);
    ksp("kernel address %lx \n", (uint64_t)kfile->address);
    ksp("kernel path %s \n", kfile->path);

    (void)kadd_request;
}
