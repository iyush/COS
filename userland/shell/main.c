#include "../../kernel/src/kstring.h"
#include "../../kernel/src/kstring.c"

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

int getchar() {
    u64 c;
    __asm__ volatile(
        "syscall"
        : "=a"(c)
        : "a"(2)
        :
    );
    return (int)c;
}

void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}

void sysexit() {
    __asm__ volatile(
        "syscall"
        : : "a"(0)
        :
    );
}

int main (int argc, char** argv) {
    printf("Type something and press Enter (max 80 chars):\n");
    char buf[81];
    int i = 0;
    while (i < 80) {
        int c = getchar();
        if (c == '\n' || c == '\r') break;
        if (c == 0) continue; // no input yet
        buf[i++] = (char)c;
        printf("%c", (char)c); // echo
    }
    buf[i] = '\0';
    printf("\n");
    to_upper(buf);
    printf("Transformed: %s\n", buf);
    return 0;
}
