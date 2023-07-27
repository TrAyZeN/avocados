
#include "instr.h"
#include "serial.h"

/*
 * Documentation:
 * https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming
 */

// Divisor Latch Low Byte (WARN: Only active when DLAB = 1)
#define SERIAL_REG_DLL(base) (base)
// Divisor Latch High Byte (WARN: Only active when DLAB = 1)
#define SERIAL_REG_DLH(base) ((base) + 1)
// Interrupt Enable Register
#define SERIAL_REG_IER(base) ((base) + 1)
// Interrupt Identification Register
#define SERIAL_REG_IIR(base) ((base) + 2)
// FIFO Control Register
#define SERIAL_REG_FCR(base) ((base) + 2)
// Line Control Register
#define SERIAL_REG_LCR(base) ((base) + 3)
// Modem Control Register
#define SERIAL_REG_MCR(base) ((base) + 4)
// Line Status Register
#define SERIAL_REG_LSR(base) ((base) + 5)
// Modem Status Register
#define SERIAL_REG_MSR(base) ((base) + 6)

static void serial_set_baudrate(enum serial_port port,
                                enum serial_baudrate baudrate);

void serial_init(enum serial_port port, enum serial_baudrate baudrate) {
    serial_set_baudrate(port, baudrate);

    /*
     * Line Control Register bits:
     * 7   - Divisor Latch Access Bit (DLAB)
     * 6   - Set break disable
     * 3:5 - Parity, 0b000 corresponds to no parity
     * 2   - Number of stop bits, 0 corresponds to one stop bit
     * 0:1 - Word length, 0b11 corresponds to 8 bits word length
     */
    outb(SERIAL_REG_LCR(port), 0x03);

    /*
     * FIFO Control Register bits:
     * 7:6 - 14 bytes trigger interrupt threshold
     * 5   - Disable 64 byte FIFO
     * 4   - Reserved
     * 3   - DMA mode select...
     * 2   - Clear transmit FIFO
     * 1   - Clear receive FIFO
     * 0   - Enable FIFO
     */
    outb(SERIAL_REG_FCR(port), 0xc7);

    /*
     * Interrupt Enable Register bits:
     * 1 - Enable transmitter holding register empty interrupt
     */
    outb(SERIAL_REG_IER(port), (1 << 1));
}

static void serial_set_baudrate(enum serial_port port,
                                enum serial_baudrate baudrate) {
    // Set DLAB to 1, to be able to modify DLL and DLH
    outb(SERIAL_REG_LCR(port), (1 << 7));

    outb(SERIAL_REG_DLL(port), baudrate & 0xff);
    outb(SERIAL_REG_DLH(port), (baudrate >> 8) & 0xff);

    // Set DLAB to 0, to be able to read and write data
    outb(SERIAL_REG_LCR(port), 0x00);
}

bool serial_can_transmit(enum serial_port port) {
    /*
     * Line Status Register bits:
     * 5 - Empty transmitter holding register
     */
    return inb(SERIAL_REG_LSR(port)) & (1 << 5);
}

void serial_write_byte(enum serial_port port, uint8_t byte) {
    while (!serial_can_transmit(port)) {
        pause();
    }

    outb(port, byte);
}

void serial_write(enum serial_port port, const char *buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Fix line endings
        if (buf[i] == '\n') {
            serial_write_byte(port, '\r');
        }

        serial_write_byte(port, buf[i]);
    }
}

void serial_puts(enum serial_port port, const char *str) {
    while (*str != '\0') {
        // Fix line endings
        if (*str == '\n') {
            serial_write_byte(port, '\r');
        }

        serial_write_byte(port, *str);

        str += 1;
    }
}
