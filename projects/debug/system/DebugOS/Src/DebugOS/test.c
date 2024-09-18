#include "DebugOS.h"

#include "stdio.h"
#include "gr55xx_delay.h"

__align(16) static uint32_t task_a_stack[2048];
__align(16) static uint32_t task_b_stack[2048];
__align(16) static uint32_t task_c_stack[2048];

void task_a_func(void * p_arg) {
    int count = 0 ;
    printf("Task A Start ...\r\n");
    while(1) {
        delay_ms(100);
        printf("Task A Running %d ...\r\n", count++);
    }
}

void task_b_func(void * p_arg) {
    int count = 10 ;
    printf("Task B Start ...\r\n");
    while(1) {
        delay_ms(100);
        printf("Task b Running %d ...\r\n", count++);
    }
}

void task_c_func(void * p_arg) {
    int count = 20 ;
    printf("Task C Start ...\r\n");
    while(1) {
        delay_ms(100);
        printf("Task C Running %d ...\r\n", count++);
    }
}

void os_test(void) {
    os_task_create("TaskA", 1, (void*) &task_a_stack[0], 2048, task_a_func, NULL);
    os_task_create("TaskB", 1, (void*) &task_b_stack[0], 2048, task_b_func, NULL);
    os_task_create("TaskC", 1, (void*) &task_c_stack[0], 2048, task_c_func, NULL);
    
    os_sched_start();
    
    for(;;) {}
    
    return;
}

