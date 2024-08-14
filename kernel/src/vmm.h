#ifndef VMM_H
#define VMM_H

#include <stdint.h>


typedef uint64_t VMMTableEntry;
typedef VMMTableEntry* VMMPageTable;

typedef uint64_t VMMPageAddress;
typedef uint64_t PMMFrameAddress;

#define VMM_P4 ((uint64_t*)(0xFFFF000000000000ULL | (RECURSIVE_PTE_INDEX << 39) | (RECURSIVE_PTE_INDEX << 30) | (RECURSIVE_PTE_INDEX << 21) | (RECURSIVE_PTE_INDEX << 12)))


PMMFrameAddress vmm_translate_page_addr(VMMPageAddress page_address);

#endif