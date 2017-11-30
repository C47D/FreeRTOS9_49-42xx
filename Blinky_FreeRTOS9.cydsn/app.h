#ifndef APP_H
#define APP_H

#include "FreeRTOS.h"
#include "task.h"

// tasks
portTASK_FUNCTION_PROTO(vLEDTask, arg);

// setup
void prvHardwareSetup(void);

// hooks
    
#endif

/* [] END OF FILE */
