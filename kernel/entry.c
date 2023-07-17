#include "./kio.h"
#include "./idt.h"

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1) {}
}

void c_start() {
    //char myBah [3] = "hel";

    //ksp("Static array: %s\n", myBah);
    //ksp("char c: %c\n", 'c');
    //ksp("This should be digit 9: %d\n", 9);
    //ksp("This should be digit 123123123: %d\n", 123123123);
    //ksp("This should be digit 0x%x\n", 0xdeadbeef);

    //ksp("This should be overflowed digit: %d\n", 0xfffffffe);

    //ksp("This is a msg: %s", "Hello World\n");
    //ksp("This should just print percentage: % \n", "");
    //ksp("This should be just nothing:%c\n", "");
    //ksp("This should be just string: %s and char: %c\n", "Hello ", 'c');
    //ksp("%");

    init_idt();
    for (int i = 0; i < 100; i++) {
        kprint("Hello %d ", i);
    }

    while (1) {

    }
}
