#define configUSE_TRACE_FACILITY 1
#define configGENERATE_RUN_TIME_STATS 1
#define portGET_RUN_TIME_COUNTER_VALUE() (RTC.COUNTER)
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() {}
#define configUSE_RECURSIVE_MUTEXES     1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskGetIdleTaskHandle         1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

#define configCPU_CLOCK_HZ			( ( unsigned long ) 160000000 )

/* Use the defaults for everything else */
#include_next<FreeRTOSConfig.h>
