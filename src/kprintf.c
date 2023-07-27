#include <stdbool.h>

#include "kassert.h"
#include "kprintf.h"
#include "serial.h"

static void num_to_str(uint64_t num, char *str, uint8_t base, bool upper,
                       bool num_signed);

/*
 * Print a string to the serial port COM1
 */
void puts(const char *str) {
    serial_puts(SERIAL_PORT_COM1, str);
}

void putchar(char c) {
    serial_write_byte(SERIAL_PORT_COM1, c);
}

/*
 * Minimal printf implementation
 */
void kprintf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
}

void kvprintf(const char *fmt, va_list ap) {
    // 23 because 8 is the smallest base so it has a greater log and
    // log(2**64 - 1, 8) = 22 + null terminator
    char int_buf[23];
    unsigned long long arg;
    int length_modifier = 0;

    for (uint64_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i += 1;

            switch (fmt[i]) {
            case 'l':
                if (fmt[i + 1] == 'l') {
                    length_modifier = 1;
                    i += 2;
                } else {
                    length_modifier = 2;
                    i += 1;
                }
                break;
            default:
                break;
            }

            switch (fmt[i]) {
            case 'd':
            case 'i':
                if (length_modifier == 1) {
                    arg = (unsigned long long)va_arg(ap, long);
                } else if (length_modifier == 2) {
                    arg = (unsigned long long)va_arg(ap, long long);
                } else {
                    arg = (unsigned long long)va_arg(ap, int);
                }
                num_to_str(arg, int_buf, 10, false, true);
                puts(int_buf);
                break;
            case 'o':
                if (length_modifier == 1) {
                    arg = va_arg(ap, unsigned long);
                } else if (length_modifier == 2) {
                    arg = va_arg(ap, unsigned long long);
                } else {
                    arg = va_arg(ap, unsigned);
                }
                num_to_str(arg, int_buf, 8, false, false);
                puts(int_buf);
                break;
            case 'u':
                if (length_modifier == 1) {
                    arg = va_arg(ap, unsigned long);
                } else if (length_modifier == 2) {
                    arg = va_arg(ap, unsigned long long);
                } else {
                    arg = va_arg(ap, unsigned);
                }
                num_to_str(arg, int_buf, 10, false, false);
                puts(int_buf);
                break;
            case 'x':
                if (length_modifier == 1) {
                    arg = va_arg(ap, unsigned long);
                } else if (length_modifier == 2) {
                    arg = va_arg(ap, unsigned long long);
                } else {
                    arg = va_arg(ap, unsigned);
                }
                num_to_str(arg, int_buf, 16, fmt[i] == 'X', false);
                puts(int_buf);
                break;
            case 'c':
                putchar((char)va_arg(ap, int));
                break;
            case 's': {
                const char *s = va_arg(ap, const char *);
                puts(s);
                break;
            }
            case '%':
            default:
                putchar(fmt[i]);
                break;
            }
            length_modifier = 0;
        } else {
            putchar(fmt[i]);
        }
    }
}

// str buffer must be big enough
// base must be == 8 or == 10 or == 16
static void num_to_str(uint64_t num, char *str, uint8_t base, bool upper,
                       bool num_signed) {
    static const char digits[16] = "0123456789abcdef";

    kassert(base == 8 || base == 10 || base == 16);

    uint64_t i = 0;
    bool negative = false;
    if (num == 0) {
        str[i++] = '0';
    } else if (num_signed && (int64_t)num < 0) {
        str[i++] = '-';
        num = (uint64_t)(-(int64_t)num);
        negative = true;
    }

    while (num > 0) {
        char d = digits[num % base];
        if (upper && 'a' <= d && d <= 'f') {
            d = (char)(d - 'a' + 'A');
        }

        str[i++] = d;
        num /= base;
    }
    str[i] = '\0';

    // TODO: No revert needed
    // Revert str
    if (negative) {
        for (uint64_t j = 0; j < (i - 1) / 2; j++) {
            char tmp = str[j + 1];
            str[j + 1] = str[i - j - 1];
            str[i - j - 1] = tmp;
        }
    } else {
        for (uint64_t j = 0; j < i / 2; j++) {
            char tmp = str[j];
            str[j] = str[i - j - 1];
            str[i - j - 1] = tmp;
        }
    }
}
