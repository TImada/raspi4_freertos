/* uart.c */
#include <stddef.h>

#include "FreeRTOS.h"
#include "board.h"
#include "interrupt.h"
#include "semphr.h"
#include "uart.h"

/* PL011 UART on Raspberry pi 4B */
#define UART_BASE  (0xFE201400U) /* UART2 */
#define UART_DR   (*(volatile unsigned int *)(UART_BASE))
#define UART_FR   (*(volatile unsigned int *)(UART_BASE+0x18U))
#define UART_IBRD (*(volatile unsigned int *)(UART_BASE+0x24U))
#define UART_FBRD (*(volatile unsigned int *)(UART_BASE+0x28U))
#define UART_LCRH (*(volatile unsigned int *)(UART_BASE+0x2CU))
#define UART_CR   (*(volatile unsigned int *)(UART_BASE+0x30U))
#define UART_IFLS (*(volatile unsigned int *)(UART_BASE+0x34U))
#define UART_IMSC (*(volatile unsigned int *)(UART_BASE+0x38U))
#define UART_ICR  (*(volatile unsigned int *)(UART_BASE+0x44U))

/* GPIO */
#define GPIO_BASE (0xFE200000U)
#define GPFSEL0   (*(volatile unsigned int *)(GPIO_BASE))
#define GPIO_PUP_PDN_CNTRL_REG0 (*(volatile unsigned int *)(GPIO_BASE+0xE4U))

struct UARTCTL {
    SemaphoreHandle_t tx_mux;
    QueueHandle_t     rx_queue;
};
struct UARTCTL *uartctl;

void uart_putchar(uint8_t c)
{
    xSemaphoreTake(uartctl->tx_mux, (portTickType) portMAX_DELAY);
    /* wait until tx becomes idle. */
    while ( UART_FR & (0x20) ) { }
    UART_DR = c;
    asm volatile ("isb");
    xSemaphoreGive(uartctl->tx_mux);
}
/*-----------------------------------------------------------*/

void uart_putchar_isr(uint8_t c)
{
    xSemaphoreTakeFromISR(uartctl->tx_mux, NULL);
    /* wait mini uart for tx idle. */
    while ( (UART_FR & 0x20) ) { }
    UART_DR = c;
    asm volatile ("isb");
    xSemaphoreGiveFromISR(uartctl->tx_mux, NULL);
}
/*-----------------------------------------------------------*/

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putchar((uint8_t)str[i]);
}
/*-----------------------------------------------------------*/

void uart_puthex(uint64_t v)
{
    const char *hexdigits = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putchar(hexdigits[(v >> i) & 0xf]);
}
/*-----------------------------------------------------------*/

uint32_t uart_read_bytes(uint8_t *buf, uint32_t length)
{
    uint32_t num = uxQueueMessagesWaiting(uartctl->rx_queue);
    uint32_t i;

    for (i = 0; i < num || i < length; i++) {
        xQueueReceive(uartctl->rx_queue, &buf[i], (portTickType) portMAX_DELAY);
    }

    return i;
}
/*-----------------------------------------------------------*/

void uart_isr(void)
{
    /* RX data */
    if( !(UART_FR & (0x1U << 4)) ) {
        uint8_t c = (uint8_t) 0xFF & UART_DR;
        xQueueSendToBackFromISR(uartctl->rx_queue, &c, NULL);
    }
}
/*-----------------------------------------------------------*/

/* 
 * wait_linux()
 * This is a busy loop function to wait until Linux completes GIC initialization
 */
static void wait_linux(void)
{
    wait_gic_init();
    return;
}
/*-----------------------------------------------------------*/

void uart_init(void)
{
    uint32_t r;

    /* GPIO0 GPIO1 settings for UART2 */
    r = GPFSEL0;
    r &= ~((0x7U << 3) | 0x7U);
    r |= ((0x3U << 3) | 0x3U); /* ALT4 */
    GPFSEL0 = r;

    r = GPIO_PUP_PDN_CNTRL_REG0;
    r &= ~((0x3U << 2) | 0x3U);
    GPIO_PUP_PDN_CNTRL_REG0 = r;

    /* PL011 settings with assumption of 48MHz clock */
    UART_ICR  = 0x7FFU;         /* Clears an interrupt */
    UART_IBRD = 0x1AU;          /* 115200 baud */
    UART_FBRD = 0x3U;
    UART_LCRH = ((0x3U << 5) | (0x0U << 4));    /* 8/n/1, FIFO disabled */
    UART_IMSC = (0x1U << 4);    /* RX interrupt enabled */
    UART_CR   = 0x301;          /* Enables Tx, Rx and UART */
    asm volatile ("isb");

    uartctl = pvPortMalloc(sizeof (struct UARTCTL));
    uartctl->tx_mux = xSemaphoreCreateMutex();
    uartctl->rx_queue = xQueueCreate(16, sizeof (uint8_t));

#if defined(__LINUX__)
    uart_puts("\r\nWaiting until Linux starts booting up ...\r\n");
    wait_linux();
#endif

    isr_register(IRQ_VC_UART, UART_PRIORITY, (0x1U << 0x3U), uart_isr);
    return;
}
/*-----------------------------------------------------------*/
