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

    stmdb sp!, {r0, r3}     //r3,r0����ѹ�� MSP
    mov r0, # 0x30          //�ر� 0x30 �������ȼ��ж�
    msr basepri, r0
    dsb
    isb
    bl os_task_switch       //�л���һ�� task
    mov r0, # 0
    msr basepri, r0
    ldmia sp!, {r0, r3}     //��r0, r3 ���´�msp pop

    /* The first item in pTaskCur is the task top of stack. */
    ldr r1, [ r3 ]      //��pTaskCur ȡ��ǰtask ָ��
    ldr r0, [ r1 ]      //ȡ��ǰtask �� stack top

    /* Pop the core registers. */
#if PUSH_R4_R11
    ldmia r0!, {r4-r11, r14}        //�� stack����pop r4-411, r14,��ɺ�, r0�Ĵ����պ�ָ��stack�е� R0���ڵ�λ��
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

    msr psp, r0         //�� R0 ���ڵ�λ��ָ���psp, ��ջʱ�� ���ε��� r0,r1,r2, ..., LR,PC,xPSR 
    isb

    bx r14
/* *INDENT-ON* */
}

SECTION_RAM_CODE void SVC_Handler(void) {
    os_port_svc_handler();
}

