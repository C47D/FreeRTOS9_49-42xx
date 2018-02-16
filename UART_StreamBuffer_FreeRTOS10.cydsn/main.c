#include "project.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

// tasks
static void vLEDTask (void *arg);
static void vUARTTask (void *arg);

// setup
static void prvHardwareSetup(void);

// interrupt handlers
CY_ISR_PROTO(UART_Rx_Handler);

/**
 * Stream buffers allow a stream of bytes to be passed from an
 * interrupt service routine to a task. a byte stream can be of
 * arbitrary length and does not necessarily have a beggining or
 * end.
 */
#define sbiSTREAM_BUFFER_LENGTH_BYTES   (sizeof(char) * 10)
#define sbiSTREAM_BUFFER_TRIGGER_LEVEL  ((BaseType_t)1)

static StreamBufferHandle_t xStreamBuffer = NULL;

int main(void)
{    
    prvHardwareSetup();
    
    xStreamBuffer = xStreamBufferCreate(
        sbiSTREAM_BUFFER_LENGTH_BYTES,
        /* The number of bytes that must be in the stream buffer
         * before a task that is blocked on the stream buffer to
         * wait for data is moved out of the blocked state. */
        sbiSTREAM_BUFFER_TRIGGER_LEVEL
    );

    xTaskCreate(
        (TaskFunction_t) vLEDTask,
        "LED",
        configMINIMAL_STACK_SIZE,
        (void*) 500,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    xTaskCreate(
        (TaskFunction_t) vUARTTask,
        "UART",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
    
    vTaskStartScheduler();

    while(1) {
    }
}

static void vLEDTask (void *arg)
{
    const TickType_t time = pdMS_TO_TICKS((TickType_t) arg);
    
    while(1) {
        LED_Write(~LED_Read());
        vTaskDelay(time);
    }
}

CY_ISR(UART_Rx_Handler)
{  
    char data = UART_GetChar();
    
    xStreamBufferSendFromISR(
        xStreamBuffer,
        (void*) &data,
        sizeof(char),
        NULL
    );
    
    isr_UART_Rx_ClearPending();
}

static void vUARTTask (void *arg)
{
    (void)arg;
    char data_received;
    
    while(1) {
        
        xStreamBufferReceive(
            xStreamBuffer,
            (void*) &data_received,
            sizeof(char),
            portMAX_DELAY
        );
        
        UART_PutChar(data_received);
        
    }
}

static void prvHardwareSetup(void)
{
    // Port layer functions that need to be copied into the vector table.
    extern void xPortPendSVHandler(void);
    extern void xPortSysTickHandler(void);
    extern void vPortSVCHandler(void );

    const uint8_t CORTEX_INTERRUPT_BASE = 16;
    // Handler for Cortex Supervisor Call (SVC) - Address 11 (SVCall_IRQn = -5)
    CyIntSetSysVector(CORTEX_INTERRUPT_BASE + SVCall_IRQn,
                    (cyisraddress) vPortSVCHandler);
    // Handler for Cortex PendSV Call - Address 14 (PendSV_IRQn = -2)
    CyIntSetSysVector(CORTEX_INTERRUPT_BASE + PendSV_IRQn,
                    (cyisraddress) xPortPendSVHandler);
    // Handler for Cortex SYSTICK - Address 15 (SysTick_IRQn = -1)
    CyIntSetSysVector(CORTEX_INTERRUPT_BASE + SysTick_IRQn,
                    (cyisraddress) xPortSysTickHandler);
    
    // Start-up the peripherals.
    isr_UART_Rx_StartEx(UART_Rx_Handler);
    
    CyGlobalIntEnable;
    
    UART_Start();
}

// The heap space has been execeeded.
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    LED_Write(1);
    while(1) {
    }
}

// The stack space has been overflowed
void vApplicationStackOverflowHook(void)
{
    taskDISABLE_INTERRUPTS();
    while(1) {
    }
}

/* [] END OF FILE */
