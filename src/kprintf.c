#include "kprintf.h"

#include <stdbool.h>

#include "kassert.h"
#include "serial.h"
#include "types.h"
#include "utils.h"

enum length_modifier {
    LM_NONE,
    LM_HH,
    LM_H,
    LM_L,
    LM_LL,
};

static u64 num_to_str(u64 num, char *str, u8 base, bool upper, bool num_signed);
static void pad(u64 field_length, u64 field_width, char padding_char);

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
    enum length_modifier length_modifier;
    u8 base;
    char padding_char;
    u64 field_width;
    u64 num_field_length;

    for (u64 i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i += 1;

            padding_char = ' ';
            if (fmt[i] == '0') {
                padding_char = '0';
                i += 1;
            }

            field_width = 0;
            if (fmt[i] >= '1' && fmt[i] <= '9') {
                while (fmt[i] >= '0' && fmt[i] <= '9') {
                    field_width = field_width * 10 + (fmt[i] - '0');
                    i += 1;
                }
            }

            switch (fmt[i]) {
            case 'h':
                if (fmt[i + 1] == 'h') {
                    length_modifier = LM_HH;
                    i += 2;
                } else {
                    length_modifier = LM_H;
                    i += 1;
                }
                break;
            case 'l':
                if (fmt[i + 1] == 'l') {
                    length_modifier = LM_LL;
                    i += 2;
                } else {
                    length_modifier = LM_L;
                    i += 1;
                }
                break;
            default:
                length_modifier = LM_NONE;
                break;
            }

            switch (fmt[i]) {
            case 'd':
            case 'i':
            case 'u':
            case 'o':
            case 'x':
            case 'X':
                switch (fmt[i]) {
                case 'd':
                case 'i':
                case 'u':
                    base = 10;
                    break;
                case 'o':
                    base = 8;
                    break;
                case 'x':
                case 'X':
                    base = 16;
                    break;
                }

                switch (length_modifier) {
                case LM_NONE:
                // Value promotable to int are promoted to int
                case LM_HH:
                case LM_H:
                    if (fmt[i] == 'd' || fmt[i] == 'i') {
                        arg = (unsigned long long)va_arg(ap, int);
                    } else {
                        arg = (unsigned long long)va_arg(ap, unsigned int);
                    }
                    break;
                case LM_L:
                    if (fmt[i] == 'd' || fmt[i] == 'i') {
                        arg = (unsigned long long)va_arg(ap, long);
                    } else {
                        arg = (unsigned long long)va_arg(ap, unsigned long);
                    }
                    break;
                case LM_LL:
                    if (fmt[i] == 'd' || fmt[i] == 'i') {
                        arg = (unsigned long long)va_arg(ap, long long);
                    } else {
                        arg =
                            (unsigned long long)va_arg(ap, unsigned long long);
                    }
                    break;
                }

                num_field_length = num_to_str(arg, int_buf, base, fmt[i] == 'X',
                                              fmt[i] == 'd' || fmt[i] == 'i');
                pad(num_field_length, field_width, padding_char);
                puts(int_buf);
                break;
            case 'c':
                pad(1, field_width, padding_char);
                putchar((char)va_arg(ap, int));
                break;
            case 's': {
                const char *s = va_arg(ap, const char *);
                pad(strlen(s), field_width, padding_char);
                puts(s);
                break;
            }
            case '%':
            default:
                // TODO: Handle modifiers here
                putchar(fmt[i]);
                break;
            }
        } else {
            putchar(fmt[i]);
        }
    }
}

// str buffer must be big enough
// base must be == 8 or == 10 or == 16
static u64 num_to_str(u64 num, char *str, u8 base, bool upper,
                      bool num_signed) {
    static const char digits[16] = "0123456789abcdef";

    kassert(base == 8 || base == 10 || base == 16);

    u64 i = 0;
    bool negative = false;
    if (num == 0) {
        str[i++] = '0';
    } else if (num_signed && (i64)num < 0) {
        str[i++] = '-';
        num = (u64)(-(i64)num);
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
        for (u64 j = 0; j < (i - 1) / 2; j++) {
            char tmp = str[j + 1];
            str[j + 1] = str[i - j - 1];
            str[i - j - 1] = tmp;
        }
    } else {
        for (u64 j = 0; j < i / 2; j++) {
            char tmp = str[j];
            str[j] = str[i - j - 1];
            str[i - j - 1] = tmp;
        }
    }

    return i;
}

static void pad(u64 field_length, u64 field_width, char padding_char) {
    if (field_width > field_length) {
        for (u64 i = 0; i < field_width - field_length; ++i) {
            putchar(padding_char);
        }
    }
}
