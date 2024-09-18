#include "gr55xx.h"
#include "DebugOS.h"


void os_port_systick_init(void) {

    SysTick->LOAD  = (uint32_t)(SystemCoreClock/10 - 1UL);            /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

    NVIC_EnableIRQ(SysTick_IRQn);
}

__asm void os_port_task_start(void) {
    /* *INDENT-OFF* */
    PRESERVE8
#if 0
    /* Use the NVIC offset register to locate the stack. */
    ldr r0, =0xE000ED08
    ldr r0, [ r0 ]
    ldr r0, [ r0 ]
    /* Set the msp back to the start of the stack. */
    msr msp, r0
#endif
    /* Clear the bit that indicates the FPU is in use in case the FPU was used
     * before the scheduler was started - which would otherwise result in the
     * unnecessary leaving of space in the SVC stack for lazy saving of FPU
     * registers. */
    mov r0, #0
    msr control, r0
    /* Globally enable interrupts. */
    cpsie i
    cpsie f
    dsb
    isb

    /* Call SVC to start the first task. */
    svc 0

    nop
    nop
    /* *INDENT-ON* */
}

__asm void os_port_svc_handler(void) {
    extern pTaskCur;

    /* *INDENT-OFF* */
    PRESERVE8

    /* Get the location of the current TCB. */
    ldr r3, = pTaskCur
    ldr r1, [ r3 ]
    ldr r0, [ r1 ]
    /* Pop the core registers. */
#if PUSH_R4_R11
    ldmia r0 !, {r4-r11,r14}
#else
    /* orelse, pop the EXEC_RETURN to R14 */
    ldr r14, [r0]
    add r0, r0, #4
#endif
    msr psp, r0
    isb
    mov r0, # 0
    msr basepri, r0
    bx r14
    /* *INDENT-ON* */
}



