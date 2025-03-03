#define ASAOS 1


#if ASAOS
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

int pow(int base, int exp) {
	int result = 1;
	for (int i = 0; i < exp; i++) {
		result *= base;
	}
	return result;
}

int exit(int status) {
	(void) status;
	sysexit();
}
#else
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;

#endif


int str_to_int(char * str, u64 len, u64 base) {
	int result = 0;

	for (u64 i = 0; i < len; i++) {
		if (str[i] >= 48 && str[i] <= 57) {
			int indiv_int = str[i] - 48;
			int factor = pow(base, len - i - 1);
			result += indiv_int * factor;
		} else {
			break;
		}
	}
	return result;
}

int main (int argc, char** argv) {
	if (argc < 3) {
		printf("./hello-world <str> <ncount>\n");
		exit(1);
	}
	// printf("%s %s %s\n", argv[0], argv[1], argv[2]);
	char * str = argv[1];
	int ncount = str_to_int(argv[2], strlen(argv[2]), 10);

	int i = 0;
	while (i < ncount) {
		printf("%s %d\n", str, i + 1); 
		i++;
	}
}
