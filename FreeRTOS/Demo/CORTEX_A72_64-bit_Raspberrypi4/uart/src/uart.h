#include <stdint.h>

#define UART_PRIORITY (0xA0)

void uart_putchar(uint8_t c);
void uart_puts(const char* str);
void uart_puthex(uint64_t v);
uint32_t uart_read_bytes(uint8_t *buf, uint32_t length);
void uart_init(void);

