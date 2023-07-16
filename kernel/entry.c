#include <stdarg.h>
//#include <assert.h>
//#include <math.h>
//#include <string.h>
#include "./kstring.h"

void ksp(char * f_str, ...);
void output_to_console(char * str, int size) {
  asm volatile(
      "mov %0, %%esi\n"    // Load the address of myString into ESI register
      "mov $0x402, %%dx\n" // Example port number for serial port (COM1)
      "cld\n"              // Set the direction flag to forward
      "mov %1, %%ecx\n"   // Length of the string excluding null character
      "rep outsb\n"        // Output the string
      :
      : "g"(str), "r"(size)// Input constraint: the address of myString
      : "esi", "edx", "ecx" // Clobbered registers
  );
}


void ksp(char * f_str, ...) {
    va_list args;
    va_start(args, f_str);

    char buffer[BUF_MAX];
    
    int size = kvsprintf(buffer, BUF_MAX, f_str, args);

    output_to_console(buffer, size);
    va_end(args);
}


void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1) {}
}
                

void c_start() {
    char myBah [3] = "hel";

    ksp("Static array: %s\n", myBah);
    ksp("char c: %c\n", 'c');
    ksp("This should be digit 9: %d\n", 9);
    ksp("This should be digit 123123123: %d\n", 123123123);
    ksp("This should be digit 0x%x\n", 0xdeadbeef);

    ksp("This should be overflowed digit: %d\n", 0xfffffffe);

    ksp("This is a msg: %s", "Hello World\n");
    ksp("This should just print percentage: % \n", "");
    ksp("This should be just nothing:%c\n", "");
    ksp("This should be just string: %s and char: %c\n", "Hello ", 'c');
    ksp("%");

    //output_to_console(myString, 7);

    while (1) {

    }
}
