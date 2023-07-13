#include <stdarg.h>
#include <assert.h>
#include <math.h>

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



#define BUF_MAX 256

static int str_from_int(int val, int base, char* buffer, int buffer_offset) {
    char sec_buffer[BUF_MAX] = {0};
    int j = BUF_MAX - 2;
    for (; val && j; --j, val /= base) {
        sec_buffer[j] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
    }

    for(int k = j; k <= BUF_MAX ; k++) {
        buffer[buffer_offset] = sec_buffer[k];
        buffer_offset++;
    }


    // returns new buffer offset
    return buffer_offset;
}

int kvsprintf(char* buffer, int buffer_size, char * f_str, va_list args) {
    int i = 0;
    int curr_str_idx = 0;
    while (f_str[i] != '\0') {
        if (f_str[i] != '%') { 
            // default
            buffer[curr_str_idx] = f_str[i];
            curr_str_idx++;
        } else {
            switch (f_str[i+1]) {
                case 'c':
                    {
                        char to_put = va_arg(args, int);

                        buffer[curr_str_idx] = to_put;
                        curr_str_idx++;
                        i++;
                        break;
                    }
                case 's':
                    {
                        char * to_put = va_arg(args, char *);

                        // we append
                        int j = 0;
                        while (to_put[j] != '\0') {
                            buffer[curr_str_idx] = to_put[j];
                            curr_str_idx++;
                            j++;
                        }

                        // consume the s
                        i++;
                        break;
                    }
                
                case 'x':
                    {
                        int val = va_arg(args, int);
                        if (val < 0) {
                            buffer[curr_str_idx] = '-';
                            curr_str_idx++;
                            val *= -1;
                        }
                        curr_str_idx = str_from_int(val, 16, buffer, curr_str_idx);
                        i++;
                        break;
                    }

                case 'd':
                    {
                        int val = va_arg(args, int);
                        if (val < 0) {
                            buffer[curr_str_idx] = '-';
                            curr_str_idx++;
                            val *= -1;
                        }

                        curr_str_idx = str_from_int(val, 10, buffer, curr_str_idx);
                        
                        i++;
                        break;
                    }

                default:
                    buffer[curr_str_idx] = f_str[i];
                    curr_str_idx++;
                    break;
            }
            if (f_str[i+1] == 's') {
            } else {

            }
        }
        assert(curr_str_idx < buffer_size);
        i++;
    }
    return curr_str_idx;
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
    char *myString = "Hello world";
    char myBah [3] = "hel";

    ksp("Static array: %s\n", myBah);
    ksp("char c: %c\n", 'c');
    ksp("This should be digit 9: %d\n", 9);
    ksp("This should be digit 123123123: %d\n", 123123123);
    ksp("This should be digit 0x%x\n", 0xdeadbeef);

    int value = 0xfffffffe;
    ksp("This should be overflowed digit: %d\n", value );

    ksp("This is a msg: %s", "Hello World\n");
    ksp("This should just print percentage: % \n", "");
    ksp("This should be just nothing:%c\n", "");
    ksp("This should be just string: %s and char: %c\n", "Hello ", 'c');
    ksp("%");
    assert(0);
    //output_to_console(myString, 7);

    while (1) {

    }
}
