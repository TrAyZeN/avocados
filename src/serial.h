#ifndef AVOCADOS_SERIAL_H_
#define AVOCADOS_SERIAL_H_

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

// COM ports port IO addresses
enum serial_port : u16 {
    SERIAL_PORT_COM1 = 0x3f8,
    SERIAL_PORT_COM2 = 0x2f8,
    SERIAL_PORT_COM3 = 0x3e8,
    SERIAL_PORT_COM4 = 0x2e8,
};

enum serial_baudrate {
    SERIAL_BAUDRATE_115200 = 0x0001,
    SERIAL_BAUDRATE_57600 = 0x0002,
    SERIAL_BAUDRATE_38400 = 0x0003,
    SERIAL_BAUDRATE_19200 = 0x0006,
    SERIAL_BAUDRATE_9600 = 0x000c,
    SERIAL_BAUDRATE_4800 = 0x0018,
    SERIAL_BAUDRATE_2400 = 0x0030,
    SERIAL_BAUDRATE_1200 = 0x0060,
    SERIAL_BAUDRATE_600 = 0x00c0,
    SERIAL_BAUDRATE_300 = 0x0180,
    SERIAL_BAUDRATE_220 = 0x020c,
    SERIAL_BAUDRATE_110 = 0x0417,
    SERIAL_BAUDRATE_50 = 0x0900,
};

void serial_init(enum serial_port port, enum serial_baudrate baudrate);

bool serial_can_transmit(enum serial_port port);

void serial_write_byte(enum serial_port port, u8 byte);
void serial_write(enum serial_port port, const char *buf, size_t count);
void serial_puts(enum serial_port port, const char *str);

#endif /* ! AVOCADOS_SERIAL_H_ */
