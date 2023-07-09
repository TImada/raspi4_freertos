#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "uart.h"

/* 
 * Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file.
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );

static inline void io_halt(void)
{
    asm volatile ("wfi");
    return;
}
/*-----------------------------------------------------------*/

void TaskA(void *pvParameters)
{
    (void) pvParameters;

    for( ;; )
    {
        uart_puts("\r\n");
        uart_puthex(xTaskGetTickCount());
        uart_puts("\r\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    return; /* Never reach this line */
}

/*-----------------------------------------------------------*/

TimerHandle_t timer;
uint32_t count=0;
void interval_func(TimerHandle_t pxTimer)
{
    (void) pxTimer;
    uint8_t buf[2] = {0};
    uint32_t len = 0;

    len = uart_read_bytes(buf, sizeof(buf) - 1);
    if (len)
        uart_puts((char *)buf);

    return;
}
/*-----------------------------------------------------------*/

void main(void)
{
    TaskHandle_t task_a;

    uart_init();
    uart_puts("\r\n****************************\r\n");
    uart_puts("\r\n    FreeRTOS UART Sample\r\n");
    uart_puts("\r\n  (This sample uses UART2)\r\n");
    uart_puts("\r\n****************************\r\n");

    xTaskCreate(TaskA, "Task A", 512, NULL, 0x10, &task_a);

    timer = xTimerCreate("print_every_10ms",(10 / portTICK_RATE_MS), pdTRUE, (void *)0, interval_func);
    if(timer != NULL)
    {
        xTimerStart(timer, 0);
    }

    vTaskStartScheduler();
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
