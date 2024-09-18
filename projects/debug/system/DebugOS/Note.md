# 要点说明



### 1. 任务栈初始化的帧结构

任务栈初始化函数如下:

```c
static uint32_t * initTaskStack(uint32_t * pStackTop, TaskFunc_p p_func, void * p_arg) {
    /***********************************************
     * 初始化栈, 一共消耗18 WORD (10个必须), 依次为:
     *      前10个: 栈边界MAGIC NUMBER, xPSR, PC, LR, R12, R3, R2, R1, R0, EXEC_RETURN  
     *      后8个 : 如果压栈 R4 ~ R11
     ***********************************************/

    *pStackTop = 0xDEADBEAF;
    
     pStackTop --;
    *pStackTop = OS_INIT_XPSR;                                  //xPSR
    
     pStackTop --;
    *pStackTop = ((uint32_t) p_func) & 0xFFFFFFFE;              //PC
    
     pStackTop --;
    *pStackTop = ((uint32_t) taskExitError) & 0xFFFFFFFE;       //LR
    
     pStackTop --;                                              //R12
     pStackTop --;                                              //R3
     pStackTop --;                                              //R2
     pStackTop --;                                              //R1

     pStackTop --;
    *pStackTop = (uint32_t) p_arg;                              //R0
    
     pStackTop --;
    *pStackTop = 0xFFFFFFFD;                                    //EXEC_RETURN
    
#if PUSH_R4_R11
    pStackTop -= 8;
#endif
    
    return pStackTop;
}
```

- 配合 os_port_svc_handler 阅读:

```
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
```

- 初始化了一个栈帧:

  - 将 0xFFFFFFFD 给 R14, 定义返回行为为 返回PSP 线程模式
  - 将 R0 所在的地址赋值给 PSP 寄存器. 则 POP Stack 时候 出栈顺序为 R0, R1,R2,R3, R12, LR, PC, xPSR, 跟Cortex-M4 定义的帧结构对应

  ![1726627808582](.\img\stack_frame.png)