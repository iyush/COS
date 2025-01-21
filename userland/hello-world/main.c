#include "../../kernel/src/kstring.h"
#include "../../kernel/src/kstring.c"

/*
 * Syscall interface
 * %rax -> syscall number
 * Arguments:
 * %rdi, %rsi, %rdx, %r10, %r8 and %r9.
 *
 * Look at the gnu manual for gcc machine machine constraints.
 * https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html
 */

extern void sysexit() {
		__asm__ volatile(
				"syscall"
				: : "a"(0)
				:
				);
}

__attribute__((format(printf, 1, 2)))
void printf(char * f_str, ...) {
    va_list args;
    va_start(args, f_str);

    char buffer[BUF_MAX];
    
    int size = kvsprintf(buffer, f_str, args);

		__asm__ volatile(
				"syscall"
				: : "a"(1), "D"(buffer), "S"(size)
				:
				);
    va_end(args);
}


int main () {
	char * result = "Hello world!";
	int total = 0;
	while (!result) {
		total += *result;
		result++;
	}
	printf("%s\n", result);
	return total;
}
