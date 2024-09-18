#include "DebugOS.h"


Task_t * pTaskHead = NULL;
Task_t * pTaskCur  = NULL;


static void taskExitError(void) {
    for(;;){}
}

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

void * os_task_create(char * name, uint32_t prio, void * p_stack, uint32_t stack_size, TaskFunc_p func, void * p_arg) {
    Task_t *   p_temp      = NULL;
    Task_t *   p_node      = malloc(sizeof(Task_t));

        memset(p_node, 0, sizeof(Task_t));
    p_node->name = malloc(strlen(name) + 1);
        memset(p_node->name, 0, strlen(name) + 1);
        memcpy(p_node->name, name, strlen(name));
    
    p_node->prio         = prio;
    p_node->p_stack_min  = p_stack;
    p_node->p_stack_max  = (uint32_t*)p_stack + (stack_size - 1);
    p_node->p_stack_size = stack_size;
    p_node->p_func       = func;
    p_node->p_stack_top  = initTaskStack(p_node->p_stack_max, func, p_arg);
    p_node->p_next       = NULL;
    
    if(pTaskHead == NULL) {
        pTaskHead = p_node;
    } else {
        p_temp = pTaskHead;
        
        while(p_temp->p_next) {
            p_temp = p_temp->p_next;
        }
        
        p_temp->p_next = p_node;
    }
    
    return (void*) p_node;
}

void os_task_switch(void) {
    if(pTaskCur->p_next) {
        pTaskCur = pTaskCur->p_next;
    } else {
        pTaskCur = pTaskHead;
    }
}