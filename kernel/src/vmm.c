#include <pmm.h>
#include <stdint.h>
#include <limine.h>
#include <mem.h>

extern uint64_t _KERNEL_START;
extern uint64_t _KERNEL_END;

#define RECURSIVE_PAGE_TABLE_ENTRY_OFFSET 510

uint64_t pml4_table[512][512] __attribute__((aligned (4096)));

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

void * vmm_get_pdpte(uint64_t* pml4e_table, uint64_t pml4e_offset)
{
    return 

}

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

    if (!(pml4e_table[pml4e_offset] & PAGE_PRESENT))
    {
        pml4e_table[pml4e_offset] = (uint64_t)(pmm_alloc_page(1)) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    uint64_t * pdpte_table = vmm_get_pdpte(pml4e_table, pml4e_offset); // we can't do this as the page table is not in the cr4.

    uint64_t * pd_table;
    if (pdpte_table[pdpte_offset] & PAGE_PRESENT)
    {
        pd_table = (uint64_t *)((pdpte_table[pdpte_offset] >> 12) * 4096);
    }
    else
    {
        pd_table = pmm_alloc_page(1);
        pdpte_table[pdpte_offset] = (uint64_t)(pd_table) | PAGE_PRESENT | PAGE_WRITABLE; 
    }

    uint64_t * p_table;
    if (pd_table[pde_offset] & PAGE_PRESENT)
    {
        p_table = (uint64_t *)((pd_table[pde_offset] >> 12) * 4096);
    }
    else
    {
        p_table = pmm_alloc_page(1);
        pd_table[pde_offset] = (uint64_t)(p_table) | PAGE_PRESENT | PAGE_WRITABLE; 
    }

    p_table[pte_offset] = (uint64_t)p_start | PAGE_PRESENT | PAGE_WRITABLE;
}

void vmm_init(struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kadd_request)
{
    (void)hhdm_request;
    uint64_t kernel_code_size_in_pages = ((uint64_t)&_KERNEL_END - (uint64_t)&_KERNEL_START + 4096) / 4096;

    uint64_t * p4_table = pml4_table[0];
    p4_table[RECURSIVE_PAGE_TABLE_ENTRY_OFFSET] = ((uint64_t)p4_table | PAGE_PRESENT | PAGE_WRITABLE);
    vmm_map_page(p4_table, (uint64_t *)KERNEL_TEXT_VMA, (uint64_t *)kadd_request.response->physical_base);//, kernel_code_size_in_pages);

    ksp("%lx %lx %lx kernel_code_size: %lx\n",  _KERNEL_START, (uint64_t)&_KERNEL_START, kadd_request.response->physical_base, kernel_code_size_in_pages);
}

void vmm_pg_dmp(struct limine_hhdm_request hhdm_request) 
{
    uint64_t *p4_table = (uint64_t *)(hhdm_request.response->offset + _vmm_cr3()); //pml4
    for (int p4idx = 0; p4idx < 512; p4idx++)
    {
        if ((p4_table[p4idx] & PAGE_PRESENT))
        {
            uint64_t *p3_table = (uint64_t *)((p4_table[p4idx] >> 12) * 4096 + hhdm_request.response->offset); //pdp
            ksp("%lx %lx %d\n", (uint64_t)p3_table, p4_table[p4idx], p4idx);      

            for (int p3idx = 0; p3idx < 512; p3idx++)
            {
                if ((p3_table[p3idx] & PAGE_PRESENT))
                {
                    uint64_t *p2_table = (uint64_t *)((p3_table[p3idx] >> 12) * 4096 + hhdm_request.response->offset); //pd
                    ksp("\t%lx %lx %d\n", (uint64_t)p2_table, p3_table[p3idx], p3idx);
                    
                    for (int p2idx = 0; p2idx < 512; p2idx++)
                    {
                        if ((p2_table[p2idx] & PAGE_PRESENT))
                        {
                            uint64_t *p1_table = (uint64_t *)((p2_table[p2idx] >> 12) * 4096 + hhdm_request.response->offset);                      // pt
                            ksp("\t\t%lx %lx %d\n", (uint64_t)p1_table, p2_table[p2idx], p2idx);


                            for (int p1idx = 0; p1idx < 512; p1idx++)
                            {
                                if ((p1_table[p1idx] & PAGE_PRESENT))
                                {
                                    uint64_t* page = (uint64_t *)((p1_table[p1idx] >> 12) * 4096 + hhdm_request.response->offset);
                                    ksp("\t\t\t%lx %lx %d\n", (uint64_t)page, p1_table[p1idx], p1idx);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}