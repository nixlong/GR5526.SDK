#include "gr55xx.h"
#include "DebugOS.h"

static volatile uint32_t g_sys_tick = 0;
static volatile uint32_t g_last_sched_tick = 0;

SECTION_RAM_CODE void __trigger_pendsv(void) {
    #define SCB_ICSR                    (*((volatile unsigned int *)(0xE000ED04)))
    #define SCB_ICSR_PENDSV_SET_BIT     (1UL << 28)
    
    SCB_ICSR = SCB_ICSR_PENDSV_SET_BIT;
}

SECTION_RAM_CODE void SysTick_Handler(void) {
    g_sys_tick++;
    
    if(g_sys_tick >= g_last_sched_tick + SCHEDULE_TICK) {
        g_last_sched_tick = g_sys_tick;
        
        __trigger_pendsv();
    }
}

__asm SECTION_RAM_CODE void PendSV_Handler(void)
{
    extern pTaskCur;
    extern os_task_switch;

/* *INDENT-OFF* */
    PRESERVE8

    mrs r0, psp
    isb
    /* Get the location of the current TCB. */
    ldr r3, =pTaskCur
    ldr r2, [ r3 ]

#if 1
    /* Is the task using the FPU context?  If so, push high vfp registers. */
    tst r14, #0x10
    it eq
    vstmdbeq r0!, {s16-s31}
#endif

    /* Save the core registers. */
#if PUSH_R4_R11
    stmdb r0!, {r4-r11, r14}
#else
    sub r0, r0, #4
    str r14, [ r0 ]
#endif

    /* Save the new top of stack into the first member of the TCB. */
    str r0, [ r2 ]

    stmdb sp!, {r0, r3}     //r3,r0依次压入 MSP
    mov r0, # 0x30          //关闭 0x30 以上优先级中断
    msr basepri, r0
    dsb
    isb
    bl os_task_switch       //切换下一个 task
    mov r0, # 0
    msr basepri, r0
    ldmia sp!, {r0, r3}     //将r0, r3 重新从msp pop

    /* The first item in pTaskCur is the task top of stack. */
    ldr r1, [ r3 ]      //从pTaskCur 取当前task 指针
    ldr r0, [ r1 ]      //取当前task 的 stack top

    /* Pop the core registers. */
#if PUSH_R4_R11
    ldmia r0!, {r4-r11, r14}        //从 stack依次pop r4-411, r14,完成后, r0寄存器刚好指向stack中的 R0所在的位置
#else
    ldr r14, [ r0 ]
    add r0, r0, #4
#endif
    
#if 1
    /* Is the task using the FPU context?  If so, pop the high vfp registers
     * too. */
    tst r14, # 0x10
    it eq
    vldmiaeq r0!, {s16-s31}
#endif

    msr psp, r0         //将 R0 所在的位置指针给psp, 弹栈时候 依次弹出 r0,r1,r2, ..., LR,PC,xPSR 
    isb

    bx r14
/* *INDENT-ON* */
}

SECTION_RAM_CODE void SVC_Handler(void) {
    os_port_svc_handler();
}

