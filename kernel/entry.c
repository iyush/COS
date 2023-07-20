#include "./kio.h"
#include "./idt.h"
#include "./io.h"

#include "./test/test_kstring.h"

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1) {}
}


uint32_t cpuid() {
    uint32_t cpuid;
    asm __volatile__ (
            "mov $0x80000001, %%eax\n"
            "cpuid\n"
            "mov %%edx, %0;"
            :"=r"(cpuid)
            );
    return cpuid;
}

void c_start() {
    test_kstring_all();
    unsigned int * x = (unsigned int * )0xdeadbeefdeadbeef;
    ksp("%x\n", cpuid());
    ksp("%p\n", x);
    cpuid();

    init_idt();

    while (1) {

    }
}
