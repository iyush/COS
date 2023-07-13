// A very basic string lib with static mem
// no mallocs here.
// Therefore quite portable
// Created on: 13.07.23 22:25
//
#include <stdarg.h>

#define BUF_MAX 256
static int str_from_uint(unsigned int val, unsigned int base, char* buffer, int buffer_offset) {
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
                        char to_put = (char)va_arg(args, int);

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
                        curr_str_idx = str_from_uint((unsigned int) val, 16, buffer, curr_str_idx);
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

                        curr_str_idx = str_from_uint((unsigned int) val, 10, buffer, curr_str_idx);
                        
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


