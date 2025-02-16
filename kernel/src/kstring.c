// A very basic string lib with static mem
// no mallocs here.
// Therefore quite portable
// Created on: 13.07.23 22:25
//

#include <stdarg.h>
#include <stdbool.h>
#include "./stdint.h"
#include "./kstring.h"

#define BUF_MAX 256

static int str_from_uint(unsigned int val, unsigned int base, char* buf, int buf_offset) {
    char sec_buf[BUF_MAX] = {0};
    int j = BUF_MAX - 2;
    for (; val && j; --j, val /= base) {
        sec_buf[j] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
    }

    for(int k = j + 1; k < BUF_MAX-1 ; k++) {
        buf[buf_offset] = sec_buf[k];
        buf_offset++;
    }

    // returns new buf offset
    return buf_offset;
}

static int str_from_ulongint(unsigned long int val, unsigned long int base, char* buf, int buf_offset) {
    char sec_buf[BUF_MAX] = {0};
    int j = BUF_MAX - 2;
    for (; val && j; --j, val /= base) {
        sec_buf[j] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
    }

    for(int k = j + 1; k < BUF_MAX-1 ; k++) {
        buf[buf_offset] = sec_buf[k];
        buf_offset++;
    }

    // returns new buf offset
    return buf_offset;
}

// make this a size_t
int strlen(const char* s) {
    if (!*s) {
        return 0;
    }
    int size = 0;
    while (*s != '\0') {
        size++;
        s++;
    }
    return size;
}

int strcmp(const char * s1, char * s2) {
    int i = 0;
    int ret_val = 0;


    if (strlen(s1) < strlen(s2)) {
        return -1;
    } else if (strlen(s1) > strlen(s2)) {
        return 1;
    }

    while (s1[i] != '\0' && s2[i] != '\0' ) {
        int diff = s1[i] - s2[i];
        if (diff > 0) {
            ret_val = 1;
            break;
        } else if (diff < 0) {
            ret_val = -1;
            break;
        }
        i++;
    }
    return ret_val;
}

static void _parse_inner_string(char * f_str, char * buf, int * curr_str_idx, int * i, bool is_long, va_list args) {
    switch (f_str[*i+1]) {
        case '%':
            {
                buf[*curr_str_idx] = '%';
                (*curr_str_idx)++;
                (*i)++;
                break;
            }
        case 'c':
            {
                char to_put = (char)va_arg(args, int);

                buf[*curr_str_idx] = to_put;
                (*curr_str_idx)++;
                (*i)++;
                break;
            }
        case 's':
            {
                char * to_put = va_arg(args, char *);

                // we append
                int j = 0;
                while (to_put[j] != 0) {
                    buf[*curr_str_idx] = to_put[j];
                    (*curr_str_idx)++;
                    j++;
                }

                // consume the s
                (*i)++;
                break;
            }

        case 'x':
            {
                // NOTE: for some reason if we are passing x, we should convert it to unsigned.
                // this is also what c compiler does
                // https://godbolt.org/z/cTPareMMn
                if (is_long) {
                    unsigned long int val = va_arg(args, unsigned long int);
                    *curr_str_idx = str_from_ulongint(val, 16, buf, *curr_str_idx);
                } else {
                    unsigned int val = va_arg(args, unsigned int);
                    *curr_str_idx = str_from_uint(val, 16, buf, *curr_str_idx);
                }

                (*i)++;
                break;
            }

        case 'p':
            {
                buf[*curr_str_idx] = '0';
                (*curr_str_idx)++;
                buf[*curr_str_idx] = 'x';
                (*curr_str_idx)++;
                unsigned long int val = va_arg(args, unsigned long int);
                *curr_str_idx = str_from_ulongint(val, 16, buf, *curr_str_idx);
                (*i)++;
                break;
            }

        case 'b':
            {
                // NOTE: this is not standard, other c compilers does not have a binary mode!
                if (is_long) {
                    unsigned long int val = va_arg(args, unsigned long int);
                    *curr_str_idx = str_from_ulongint(val, 2, buf, *curr_str_idx);
                } else {
                    unsigned int val = va_arg(args, unsigned int);
                    *curr_str_idx = str_from_uint(val, 2, buf, *curr_str_idx);
                }

                (*i)++;
                break;
            }

        case 'u':
            {
                unsigned int val = va_arg(args, unsigned int);

                if (val == 0) {
                    buf[*curr_str_idx] = '0';
                    (*curr_str_idx)++;
                    (*i)++;
                    break;
                }
                *curr_str_idx = str_from_uint((unsigned int) val, 10, buf, *curr_str_idx);

                (*i)++;
                break;
            }

        case 'd':
            {
                if (!is_long) {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        buf[*curr_str_idx] = '-';
                        (*curr_str_idx)++;
                        val *= -1;
                    }

                    if (val == 0) {
                        buf[*curr_str_idx] = '0';
                        (*curr_str_idx)++;
                        (*i)++;
                        break;
                    }
                    *curr_str_idx = str_from_uint((unsigned int) val, 10, buf, *curr_str_idx);
                } else if (is_long) {
                    long int val = va_arg(args, long int);
                    if (val < 0) {
                        buf[*curr_str_idx] = '-';
                        (*curr_str_idx)++;
                        val *= -1;
                    }

                    if (val == 0) {
                        buf[*curr_str_idx] = '0';
                        (*curr_str_idx)++;
                        (*i)++;
                        break;
                    }
                    *curr_str_idx = str_from_ulongint((unsigned long int )val, 10, buf, *curr_str_idx);
                }

                (*i)++;
                break;
            }

        default:
            buf[*curr_str_idx] = f_str[*i];
            (*curr_str_idx)++;
            break;
    }
}



int kvsprintf(char* buf, char * f_str, va_list args) {
    int i = 0;
    int curr_str_idx = 0;

    bool is_long = false;

    while (f_str[i] != '\0') {
        if (f_str[i] != '%') {
            // default
            buf[curr_str_idx] = f_str[i];
            curr_str_idx++;
        } else {
            if (f_str[i+1] == 'l') {
                is_long = true;
                i++;
                _parse_inner_string(f_str, buf, &curr_str_idx, &i, is_long, args);
                is_long = false;
            } else {
                _parse_inner_string(f_str, buf, &curr_str_idx, &i, is_long, args);
            }
        }
        i++;
    }
    va_end(args);
    return curr_str_idx;
}
