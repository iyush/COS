#include <pmm.h>
#include <stdint.h>
#include <limine.h>

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

void vmm_init(struct limine_hhdm_request hhdm_request)
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