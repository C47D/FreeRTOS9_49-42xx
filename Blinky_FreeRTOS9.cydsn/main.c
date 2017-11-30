#include "project.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app.h"

int main(void)
{    
    prvHardwareSetup();

    xTaskCreate(
        (TaskFunction_t) vLEDTask
        , "LED"
        , configMINIMAL_STACK_SIZE
        , (void*) 500
        , tskIDLE_PRIORITY + 1
        , NULL
    );
    
    vTaskStartScheduler();

    while(1) {
    }
}

/* [] END OF FILE */
