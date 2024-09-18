#include "DebugOS.h"


void os_sched_start(void) {

    pTaskCur = pTaskHead;
    
    os_port_systick_init();
    os_port_task_start();
}

