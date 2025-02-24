#pragma once

#define CPU_IA32_EFER 0xC0000080
#define CPU_IA32_STAR 0xC0000081
#define CPU_IA32_LSTAR 0xC0000082
#define CPU_IA32_FSTAR 0xC0000084

#define CPU_IA32_USER_GS_BASE 0xC0000101
#define CPU_IA32_KERNEL_GS_BASE 0xC0000102


typedef struct CpuId {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
} CpuId;

CpuId cpuid(u32 in_eax) {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    asm __volatile__ (
            "mov %4, %%eax;"
            "cpuid;"
            "mov %%eax, %0;"
            "mov %%ebx, %1;"
            "mov %%ecx, %2;"
            "mov %%edx, %3;"
            :"=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            :"r"(in_eax)
            :
            );
    return (CpuId) {
        .eax = eax,
        .ebx = ebx,
        .ecx = ecx,
        .edx = edx,
    };
}

void wrmsr(u32 msr, u64 value) {
    asm __volatile__(
        "wrmsr"
        :
        : "d"((u32)(value >> 32)), "a"((u32)(value & 0xffffffff)), "c"(msr)
        :
        );
}

u64 rdmsr(u32 msr) {
    u32 low = 0;
    u64 high = 0;

    asm __volatile__(
        "\n mov %0, %%ecx"
        "\n rdmsr"
        : "=a" (low), "=d"(high)
        : "r"(msr)
        :
        );

    return high << 32 | low;
}
