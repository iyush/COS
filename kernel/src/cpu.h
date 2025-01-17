#pragma once

#define CPU_IA32_EFER 0xC0000080
#define CPU_IA32_STAR 0xC0000081
#define CPU_IA32_LSTAR 0xC0000082

u32 cpuid() {
    u32 cpuid;
    asm __volatile__ (
            "mov $0x80000001, %%eax\n"
            "cpuid\n"
            "mov %%edx, %0;"
            :"=r"(cpuid)
            );
    return cpuid;
}

void wrmsr(u32 msr, u64 value) {
    asm __volatile__(
        "\n wrmsr"
        :
        : "edx"((u32)(value >> 32)), "eax"((u32)(value & 0xffffffff)), "ecx"(msr)
        :
        );
}

u64 rdmsr(u32 msr) {
    u32 low = 0;
    u64 high = 0;

    asm __volatile__(
        "\n mov %0, %%ecx"
        "\n rdmsr"
        : "=eax" (low), "=edx"(high)
        : "r"(msr)
        :
        );

    return high << 32 | low;
}