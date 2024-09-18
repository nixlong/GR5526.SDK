#ifndef __DEBUG_OS_H__
#define __DEBUG_OS_H__

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"


#define PUSH_R4_R11             1
#define SCHEDULE_TICK           10
#define OS_INIT_XPSR            0x01000000      /* xPSR 初值, 指令thumb 状态, 其他bit清零*/

typedef void (* TaskFunc_p)(void *);

/*
 ******* Stack Max (Bottom Boundry)
 *******     |
 *******    \_/
 *******
 ******* Stack Top
 *******     |
 *******    \_/
 *******
 ******* Stack Min (Top Boundry)
 *******
 */

typedef struct _Task_t {
    uint32_t *      p_stack_top;
    uint32_t *      p_stack_min;
    uint32_t *      p_stack_max;        //
    uint32_t        p_stack_size;
    uint32_t        prio;
    char *          name;
    TaskFunc_p      p_func;
    void *          p_handle;
    struct _Task_t * p_next;
} Task_t;

extern Task_t * pTaskHead;
extern Task_t * pTaskCur;

void os_port_systick_init(void);
__asm void os_port_task_start(void);
__asm void os_port_svc_handler(void);

void os_sched_start(void) ;

void * os_task_create(char * name, uint32_t prio, void * p_stack, uint32_t stack_size, TaskFunc_p func, void * p_arg);

#endif /* __DEBUG_OS_H__ */
