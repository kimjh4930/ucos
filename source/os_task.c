/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                            TASK MANAGEMENT
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : OS_TASK.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#ifndef  OS_MASTER_FILE
#include "includes.h"
#endif

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif
/*
*********************************************************************************************************
*                                        CHANGE PRIORITY OF A TASK
*
* Description: This function allows you to change the priority of a task dynamically.  Note that the new
*              priority MUST be available.
*
* Arguments  : oldp     is the old priority
*
*              newp     is the new priority
*
* Returns    : OS_NO_ERR        is the call was successful
*              OS_PRIO_INVALID  if the priority you specify is higher that the maximum allowed
*                               (i.e. >= OS_LOWEST_PRIO)
*              OS_PRIO_EXIST    if the new priority already exist.
*              OS_PRIO_ERR      there is no task with the specified OLD priority (i.e. the OLD task does
*                               not exist.
*********************************************************************************************************
*/

#if OS_TASK_CHANGE_PRIO_EN > 0
INT8U  OSTaskChangePrio (INT8U oldprio, INT8U newprio)
/* 태스크 우선순위 변경
*  런타임시 함수를 호출해서 동적으로 변경할 수 있음.
*/
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR    cpu_sr;
#endif

#if OS_EVENT_EN > 0
    OS_EVENT    *pevent;
#endif

    OS_TCB      *ptcb;
    INT8U        x;
    INT8U        y;
    INT8U        bitx;
    INT8U        bity;


#if OS_ARG_CHK_EN > 0
    if ((oldprio >= OS_LOWEST_PRIO && oldprio != OS_PRIO_SELF)  || /* Idle Task의 우선순위를 변경할 수 없음.*/
         newprio >= OS_LOWEST_PRIO) {
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (OSTCBPrioTbl[newprio] != (OS_TCB *)0) {                 /* 새 우선순위가 비어있는지 검사 */
        OS_EXIT_CRITICAL();
        return (OS_PRIO_EXIST);
    } else {
        OSTCBPrioTbl[newprio] = (OS_TCB *)1;                    /* 새 우선순위가 비어있다면 OSTCBPrioTbl[newprio]에 임시 값을 넣어, 해당 우선순위를 예약 */
        OS_EXIT_CRITICAL();										/* 여기서부터 인터럽트를 활성화해서 다른 태스크가 이 우선순위에 대해 태스크를 생성하거나 우선순위 변경을 요청해도 문제가 발생하지 않음.*/
        y    = newprio >> 3;                                    /* TCB내에 저장할 값을 미리 계산 */
        bity = OSMapTbl[y];
        x    = newprio & 0x07;
        bitx = OSMapTbl[x];
        OS_ENTER_CRITICAL();
        if (oldprio == OS_PRIO_SELF) {                          /* 현재 태스크가 자신의 우선순위를 변경하려 하는 것인지 검사.                */
            oldprio = OSTCBCur->OSTCBPrio;                      /* Yes, get priority                   */
        }
        ptcb = OSTCBPrioTbl[oldprio];
        if (ptcb != (OS_TCB *)0) {                              /* 요청하는 우선순위에 태스크가 존재하는지 검사          */
            OSTCBPrioTbl[oldprio] = (OS_TCB *)0;                /* 기존 우선순위에 NULL 값을 넣음.       */
            if ((OSRdyTbl[ptcb->OSTCBY] & ptcb->OSTCBBitX) != 0x00) {  /* 우선순위를 바꾸고자 하는 태스크가 실행 준비상태인지 검사 */
                if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) {
                    OSRdyGrp &= ~ptcb->OSTCBBitY;
                }
                OSRdyGrp    |= bity;                            /* Make new priority ready to run      */
                OSRdyTbl[y] |= bitx;
#if OS_EVENT_EN > 0
            } else {
                pevent = ptcb->OSTCBEventPtr;
                if (pevent != (OS_EVENT *)0) {                  /* Remove from event wait list  */
                    if ((pevent->OSEventTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0) {
                        pevent->OSEventGrp &= ~ptcb->OSTCBBitY;
                    }
                    pevent->OSEventGrp    |= bity;              /* Add new priority to wait list       */
                    pevent->OSEventTbl[y] |= bitx;
                }
#endif
            }
            OSTCBPrioTbl[newprio] = ptcb;                       /* Place pointer to TCB @ new priority */
            ptcb->OSTCBPrio       = newprio;                    /* Set new task priority               */
            ptcb->OSTCBY          = y;
            ptcb->OSTCBX          = x;
            ptcb->OSTCBBitY       = bity;
            ptcb->OSTCBBitX       = bitx;
            OS_EXIT_CRITICAL();
            OS_Sched();                                         /* Run highest priority task ready     */
            return (OS_NO_ERR);
        } else {
            OSTCBPrioTbl[newprio] = (OS_TCB *)0;                /* 요청하는 우선순위에 태스크가 존재하지 않으면 OSTCBPrioTbl[] 배열에 임의의 값을 미리 넣어서 예약해 두었던 새로운 우선순위에 대한 권리르 반납하고 에러코드 반환  */
            OS_EXIT_CRITICAL();
            return (OS_PRIO_ERR);                               /* Task to change didn't exist         */
        }
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            CREATE A TASK
*
* Description: This function is used to have uC/OS-II manage the execution of a task.  Tasks can either
*              be created prior to the start of multitasking or by a running task.  A task cannot be
*              created by an ISR.
*
* Arguments  : task     is a pointer to the task's code
*
*              pdata    is a pointer to an optional data area which can be used to pass parameters to
*                       the task when the task first executes.  Where the task is concerned it thinks
*                       it was invoked and passed the argument 'pdata' as follows:
*
*                           void Task (void *pdata)
*                           {
*                               for (;;) {
*                                   Task code;
*                               }
*                           }
*
*              ptos     is a pointer to the task's top of stack.  If the configuration constant
*                       OS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pstk' will thus point to the highest (valid) memory
*                       location of the stack.  If OS_STK_GROWTH is set to 0, 'pstk' will point to the
*                       lowest memory location of the stack and the stack will grow with increasing
*                       memory locations.
*
*              prio     is the task's priority.  A unique priority MUST be assigned to each task and the
*                       lower the number, the higher the priority.
*
* Returns    : OS_NO_ERR        if the function was successful.
*              OS_PRIO_EXIT     if the task priority already exist
*                               (each task MUST have a unique priority).
*              OS_PRIO_INVALID  if the priority you specify is higher that the maximum allowed
*                               (i.e. >= OS_LOWEST_PRIO)
*********************************************************************************************************
*/
	//첫번째 인자 : 생성하고자 하는 태스크의 시작번지
	//두번째 인자 : pdata를 생성하려는 태스크로 넘겨줄 전달인자
	//세번쨰 인자 : 생성하고자 하는 태스크에 할당한 스택 사용 시작 번지
	//네번째 인자 : 우선순위
#if OS_TASK_CREATE_EN > 0
INT8U  OSTaskCreate (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT8U prio)
{
#if OS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    OS_CPU_SR  cpu_sr;
#endif
    OS_STK    *psp;
    INT8U      err;


#if OS_ARG_CHK_EN > 0
    if (prio > OS_LOWEST_PRIO) {             /* 지정한 태스크의 우선순위가 합당한 값인지 검사           */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (OSTCBPrioTbl[prio] == (OS_TCB *)0) { /* 해당하는 태스크의 우선순위가 이미 있는지 확인. 동일한 우선순위의 태스크가 존재하면 안됨.  */
        OSTCBPrioTbl[prio] = (OS_TCB *)1;    /*지정한 우선순위에 해당 태스크가 없는 상태면 OSTCBPrioTbl[]에 임시로 넣어 같은 우선순위로 태스크가 생성될 수 없도록 예약  */
                                             /* ... the same thing until task is created.              */
        OS_EXIT_CRITICAL();					 /*여기서부터 다른 태스크가 호출한 OSTaskCreate() 함수가 동작 가능. 같은 우선순위로 태스크 생성은 불가능.*/
        psp = (OS_STK *)OSTaskStkInit(task, pdata, ptos, 0);    /* 새로운 스택 시작번지를 return. 이 값은 태스크 컨트롤 블록에 저장됨*/
        err = OS_TCBInit(prio, psp, (OS_STK *)0, 0, 0, (void *)0, 0);	/*자유 OS_TCB풀로부터 OS_TCB를 하나 할당 받아 초기화함.*/
        if (err == OS_NO_ERR) {
            OS_ENTER_CRITICAL();
            OSTaskCtr++;                     /* OS_TCBInit()의 리턴 코드 점검. 리턴 코드가 정사이면 생성된 태스크 수를관리하기 위해서 OSTaskCtr을 증가.        */
            OS_EXIT_CRITICAL();
            if (OSRunning == TRUE) {         /* Find highest priority task if multitasking has started */
                OS_Sched();
            }
        } else {
            OS_ENTER_CRITICAL();
            OSTCBPrioTbl[prio] = (OS_TCB *)0;/* return 코드가 비정상인경우 OSTCBPrioTbl[prio]를 0으로 설정해서 우선순위 예약을 해제  */
            OS_EXIT_CRITICAL();
        }
        return (err);
    }
    OS_EXIT_CRITICAL();
    return (OS_PRIO_EXIST);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                     CREATE A TASK (Extended Version)
*
* Description: This function is used to have uC/OS-II manage the execution of a task.  Tasks can either
*              be created prior to the start of multitasking or by a running task.  A task cannot be
*              created by an ISR.  This function is similar to OSTaskCreate() except that it allows
*              additional information about a task to be specified.
*
* Arguments  : task     is a pointer to the task's code
*
*              pdata    is a pointer to an optional data area which can be used to pass parameters to
*                       the task when the task first executes.  Where the task is concerned it thinks
*                       it was invoked and passed the argument 'pdata' as follows:
*
*                           void Task (void *pdata)
*                           {
*                               for (;;) {
*                                   Task code;
*                               }
*                           }
*
*              ptos     is a pointer to the task's top of stack.  If the configuration constant
*                       OS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pstk' will thus point to the highest (valid) memory
*                       location of the stack.  If OS_STK_GROWTH is set to 0, 'pstk' will point to the
*                       lowest memory location of the stack and the stack will grow with increasing
*                       memory locations.  'pstk' MUST point to a valid 'free' data item.
*
*              prio     is the task's priority.  A unique priority MUST be assigned to each task and the
*                       lower the number, the higher the priority.
*
*              id       is the task's ID (0..65535)
*
*              pbos     is a pointer to the task's bottom of stack.  If the configuration constant
*                       OS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pbos' will thus point to the LOWEST (valid) memory
*                       location of the stack.  If OS_STK_GROWTH is set to 0, 'pbos' will point to the
*                       HIGHEST memory location of the stack and the stack will grow with increasing
*                       memory locations.  'pbos' MUST point to a valid 'free' data item.
*
*              stk_size is the size of the stack in number of elements.  If OS_STK is set to INT8U,
*                       'stk_size' corresponds to the number of bytes available.  If OS_STK is set to
*                       INT16U, 'stk_size' contains the number of 16-bit entries available.  Finally, if
*                       OS_STK is set to INT32U, 'stk_size' contains the number of 32-bit entries
*                       available on the stack.
*
*              pext     is a pointer to a user supplied memory location which is used as a TCB extension.
*                       For example, this user memory can hold the contents of floating-point registers
*                       during a context switch, the time each task takes to execute, the number of times
*                       the task has been switched-in, etc.
*
*              opt      contains additional information (or options) about the behavior of the task.  The
*                       LOWER 8-bits are reserved by uC/OS-II while the upper 8 bits can be application
*                       specific.  See OS_TASK_OPT_??? in uCOS-II.H.
*
* Returns    : OS_NO_ERR        if the function was successful.
*              OS_PRIO_EXIT     if the task priority already exist
*                               (each task MUST have a unique priority).
*              OS_PRIO_INVALID  if the priority you specify is higher that the maximum allowed
*                               (i.e. > OS_LOWEST_PRIO)
*********************************************************************************************************
*/
/*$PAGE*/
#if OS_TASK_CREATE_EXT_EN > 0
INT8U  OSTaskCreateExt (void   (*task)(void *pd),
                        void    *pdata,
                        OS_STK  *ptos,
                        INT8U    prio,
                        INT16U   id,
                        OS_STK  *pbos,
                        INT32U   stk_size,
                        void    *pext,
                        INT16U   opt)
{
#if OS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    OS_CPU_SR  cpu_sr;
#endif
    OS_STK    *psp;
    INT8U      err;


#if OS_ARG_CHK_EN > 0
    if (prio > OS_LOWEST_PRIO) {             /* Make sure priority is within allowable range           */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (OSTCBPrioTbl[prio] == (OS_TCB *)0) { /* Make sure task doesn't already exist at this priority  */
        OSTCBPrioTbl[prio] = (OS_TCB *)1;    /* Reserve the priority to prevent others from doing ...  */
                                             /* ... the same thing until task is created.              */
        OS_EXIT_CRITICAL();

        if (((opt & OS_TASK_OPT_STK_CHK) != 0x0000) ||   /* See if stack checking has been enabled     */
            ((opt & OS_TASK_OPT_STK_CLR) != 0x0000)) {   /* See if stack needs to be cleared           */
            #if OS_STK_GROWTH == 1
            (void)memset(pbos, 0, stk_size * sizeof(OS_STK));
            #else
            (void)memset(ptos, 0, stk_size * sizeof(OS_STK));
            #endif
        }

        psp = (OS_STK *)OSTaskStkInit(task, pdata, ptos, opt); /* Initialize the task's stack          */
        err = OS_TCBInit(prio, psp, pbos, id, stk_size, pext, opt);
        if (err == OS_NO_ERR) {
            OS_ENTER_CRITICAL();
            OSTaskCtr++;                                       /* Increment the #tasks counter         */
            OS_EXIT_CRITICAL();
            if (OSRunning == TRUE) {                           /* Find HPT if multitasking has started */
                OS_Sched();
            }
        } else {
            OS_ENTER_CRITICAL();
            OSTCBPrioTbl[prio] = (OS_TCB *)0;                  /* Make this priority avail. to others  */
            OS_EXIT_CRITICAL();
        }
        return (err);
    }
    OS_EXIT_CRITICAL();
    return (OS_PRIO_EXIST);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            DELETE A TASK
*
* Description: This function allows you to delete a task.  The calling task can delete itself by
*              its own priority number.  The deleted task is returned to the dormant state and can be
*              re-activated by creating the deleted task again.
*
* Arguments  : prio    is the priority of the task to delete.  Note that you can explicitely delete
*                      the current task without knowing its priority level by setting 'prio' to
*                      OS_PRIO_SELF.
*
* Returns    : OS_NO_ERR           if the call is successful
*              OS_TASK_DEL_IDLE    if you attempted to delete uC/OS-II's idle task
*              OS_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                  (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_TASK_DEL_ERR     if the task you want to delete does not exist
*              OS_TASK_DEL_ISR     if you tried to delete a task from an ISR
*
* Notes      : 1) To reduce interrupt latency, OSTaskDel() 'disables' the task:
*                    a) by making it not ready
*                    b) by removing it from any wait lists
*                    c) by preventing OSTimeTick() from making the task ready to run.
*                 The task can then be 'unlinked' from the miscellaneous structures in uC/OS-II.
*              2) The function OS_Dummy() is called after OS_EXIT_CRITICAL() because, on most processors,
*                 the next instruction following the enable interrupt instruction is ignored.  
*              3) An ISR cannot delete a task.
*              4) The lock nesting counter is incremented because, for a brief instant, if the current
*                 task is being deleted, the current task would not be able to be rescheduled because it
*                 is removed from the ready list.  Incrementing the nesting counter prevents another task
*                 from being schedule.  This means that an ISR would return to the current task which is
*                 being deleted.  The rest of the deletion would thus be able to be completed.
*********************************************************************************************************
*/
/*$PAGE*/
#if OS_TASK_DEL_EN > 0
INT8U  OSTaskDel (INT8U prio)
	/*태스크를 삭제한다는 것은 태스크를 수면상태로 되돌리는 것.*/
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR     cpu_sr;
#endif

#if OS_EVENT_EN > 0
    OS_EVENT     *pevent;
#endif    
#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
    OS_FLAG_NODE *pnode;
#endif
    OS_TCB       *ptcb;
    BOOLEAN       self;

    if (OSIntNesting > 0) {         /* ISR에서 태스크를 삭제하려는 것인지 아닌지를 검사. ISR에서는 태스크를 삭제할 수 없음    */
        return (OS_TASK_DEL_ISR);
    }
#if OS_ARG_CHK_EN > 0
    if (prio == OS_IDLE_PRIO) {     /* Idle 태스크는 삭제할 수 없음.     */
        return (OS_TASK_DEL_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {       /* 통계 태스크를 포함함 모든 상위 우선순위 태스크를 삭제할 수 있음.      */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                                 /* 태스크 자기 자신도 삭제할 수 있음.    */
        prio = OSTCBCur->OSTCBPrio;                             /* Set priority to delete to current   */
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb != (OS_TCB *)0) {                                       /* 삭제하고자 하는 우선순위에 해당하는 태스크가 존재하는지 검사.    */
        if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) {  /* 태스크가 준비 리스트에 있으면 준비 리스트로부터 태스크를 삭제        */
            OSRdyGrp &= ~ptcb->OSTCBBitY;
        }

#if OS_EVENT_EN > 0				/*태스크가 mutex, mailbox, message queue, semaphore 등을 기다리기 위해 대기 리스트에서 대기중이라면, 태스크를 해당 리스트에서 삭제*/
        pevent = ptcb->OSTCBEventPtr;
        if (pevent != (OS_EVENT *)0) {                          /* If task is waiting on event         */
            if ((pevent->OSEventTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0) { /* ... remove task from */
                pevent->OSEventGrp &= ~ptcb->OSTCBBitY;                        /* ... event ctrl block */
            }
        }
#endif
#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)/*태스크가 이벤트 플래그 대기 리스트에 들어있다면 그 리스트로부터 태스크를 삭제*/
        pnode = ptcb->OSTCBFlagNode;
        if (pnode != (OS_FLAG_NODE *)0) {                       /* If task is waiting on event flag    */
            OS_FlagUnlink(pnode);                               /* Remove from wait list               */
        }
#endif
        ptcb->OSTCBDly  = 0;                                    /* 인터럽트를 활성화한 뒤, 틱 인터럽트 핸들러가 태스크를 다시 준비 리스트로 삽입하는 것을 막기 위해 TCB내의 지연 카운트 값을 0으로 설정  */
        ptcb->OSTCBStat = OS_STAT_RDY;                          /* 다른 태스크나 ISR이 OSTaskResume() 함수를 호출해서 이 태스크를 재실행하도록 하는 것을 막음.     */
		if (OSLockNesting < 255) {
            OSLockNesting++;									/* 현재 태스크는 실제 준비 리스트에서 삭제됨. 스케줄러가 다른 태스크로 문맥전환을 일으키지 않도록 스케줄러를 잠금.*/
		}
        OS_EXIT_CRITICAL();                                     /* ??이해안됨. Enabling INT. ignores next instruc. */
        OS_Dummy();                                             /* 프로세서가 인터럽트를 활성화한 상태로 적어도 하나 이상의 명령어를 수행하도록 해주기 위함.
         	 	 	 	 	 	 	 	 	 	 	 	 	 	*  대부분의 프로세서는 인터럽를 활성화 명령을 시행해도 하나 이상의 명령어가 실행되지 않으면 다음 명령이 실행할 떄까지는 여전히 인터럽트 비활성화 상태를 유지
         	 	 	 	 	 	 	 	 	 	 	 	 	 	*  OS_Dummy() 함수를 호출하면서 프로세서가 최소한 한번의 Call, Return 명령을 실행하는 것을 보장.
         	 	 	 	 	 	 	 	 	 	 	 	 	 	*/
        OS_ENTER_CRITICAL();                                    /* 스케줄러 잠금 해제                  */
		if (OSLockNesting > 0) {
            OSLockNesting--;
		}
        OSTaskDelHook(ptcb);                                    /* 사용자 정의 TCB에 대해 태스크 삭제를 위해 해주어야 할 일을 처리할 수 있음.              */
        OSTaskCtr--;                                            /* 태스크가 하나 삭제된 것을 ucos-ii에 알림         */
        OSTCBPrioTbl[prio] = (OS_TCB *)0;                       /* Clear old priority entry            */
        if (ptcb->OSTCBPrev == (OS_TCB *)0) {                   /* OSTCBList 변수가 유지하고 있는 태스크 블록 이중 연결 리스트로부터 태스크 컨트롤 블록을 제거               */
            ptcb->OSTCBNext->OSTCBPrev = (OS_TCB *)0;
            OSTCBList                  = ptcb->OSTCBNext;
        } else {
            ptcb->OSTCBPrev->OSTCBNext = ptcb->OSTCBNext;
            ptcb->OSTCBNext->OSTCBPrev = ptcb->OSTCBPrev;
        }
        ptcb->OSTCBNext = OSTCBFreeList;                        /* 삭제된 태스크 컨트롤 블록은 다시 태스크 생성을 위해 사용될 수 있도록 자유 태스크 컨트롤 블록 리스트에 삽입        */
        OSTCBFreeList   = ptcb;
        OS_EXIT_CRITICAL();
        OS_Sched();                                             /* Find new highest priority task      */
        return (OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_DEL_ERR);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                    REQUEST THAT A TASK DELETE ITSELF
*
* Description: This function is used to:
*                   a) notify a task to delete itself.
*                   b) to see if a task requested that the current task delete itself.
*              This function is a little tricky to understand.  Basically, you have a task that needs
*              to be deleted however, this task has resources that it has allocated (memory buffers,
*              semaphores, mailboxes, queues etc.).  The task cannot be deleted otherwise these
*              resources would not be freed.  The requesting task calls OSTaskDelReq() to indicate that
*              the task needs to be deleted.  Deleting of the task is however, deferred to the task to
*              be deleted.  For example, suppose that task #10 needs to be deleted.  The requesting task
*              example, task #5, would call OSTaskDelReq(10).  When task #10 gets to execute, it calls
*              this function by specifying OS_PRIO_SELF and monitors the returned value.  If the return
*              value is OS_TASK_DEL_REQ, another task requested a task delete.  Task #10 would look like
*              this:
*
*                   void Task(void *data)
*                   {
*                       .
*                       .
*                       while (1) {
*                           OSTimeDly(1);
*                           if (OSTaskDelReq(OS_PRIO_SELF) == OS_TASK_DEL_REQ) {
*                               Release any owned resources;
*                               De-allocate any dynamic memory;
*                               OSTaskDel(OS_PRIO_SELF);
*                           }
*                       }
*                   }
*
* Arguments  : prio    is the priority of the task to request the delete from
*
* Returns    : OS_NO_ERR          if the task exist and the request has been registered
*              OS_TASK_NOT_EXIST  if the task has been deleted.  This allows the caller to know whether
*                                 the request has been executed.
*              OS_TASK_DEL_IDLE   if you requested to delete uC/OS-II's idle task
*              OS_PRIO_INVALID    if the priority you specify is higher that the maximum allowed
*                                 (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_TASK_DEL_REQ    if a task (possibly another task) requested that the running task be
*                                 deleted.
*********************************************************************************************************
*/
/*$PAGE*/
#if OS_TASK_DEL_EN > 0
INT8U  OSTaskDelReq (INT8U prio)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    BOOLEAN    stat;
    INT8U      err;
    OS_TCB    *ptcb;


#if OS_ARG_CHK_EN > 0
    if (prio == OS_IDLE_PRIO) {                                 /* Not allowed to delete idle task     */
        return (OS_TASK_DEL_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {       /* Task priority valid ?               */
        return (OS_PRIO_INVALID);
    }
#endif
    if (prio == OS_PRIO_SELF) {                                 /* See if a task is requesting to ...  */
        OS_ENTER_CRITICAL();                                    /* ... this task to delete itself      */
        stat = OSTCBCur->OSTCBDelReq;                           /* Return request status to caller     */
        OS_EXIT_CRITICAL();
        return (stat);
    }
    OS_ENTER_CRITICAL();
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb != (OS_TCB *)0) {                                  /* Task to delete must exist           */
        ptcb->OSTCBDelReq = OS_TASK_DEL_REQ;                    /* Set flag indicating task to be DEL. */
        err               = OS_NO_ERR;
    } else {
        err               = OS_TASK_NOT_EXIST;                  /* Task must be deleted                */
    }
    OS_EXIT_CRITICAL();
    return (err);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                        RESUME A SUSPENDED TASK
*
* Description: This function is called to resume a previously suspended task.  This is the only call that
*              will remove an explicit task suspension.
*
* Arguments  : prio     is the priority of the task to resume.
*
* Returns    : OS_NO_ERR                if the requested task is resumed
*              OS_PRIO_INVALID          if the priority you specify is higher that the maximum allowed
*                                       (i.e. >= OS_LOWEST_PRIO)
*              OS_TASK_RESUME_PRIO      if the task to resume does not exist
*              OS_TASK_NOT_SUSPENDED    if the task to resume has not been suspended
*********************************************************************************************************
*/

#if OS_TASK_SUSPEND_EN > 0
INT8U  OSTaskResume (INT8U prio)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    OS_TCB    *ptcb;


#if OS_ARG_CHK_EN > 0
    if (prio >= OS_LOWEST_PRIO) {                               /* Make sure task priority is valid    */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb == (OS_TCB *)0) {                                  /* Task to suspend must exist          */
        OS_EXIT_CRITICAL();
        return (OS_TASK_RESUME_PRIO);
    }
    if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) != OS_STAT_RDY) {              /* Task must be suspended   */
        if (((ptcb->OSTCBStat &= ~OS_STAT_SUSPEND) == OS_STAT_RDY) &&      /* Remove suspension        */
             (ptcb->OSTCBDly  == 0)) {                                     /* Must not be delayed      */
            OSRdyGrp               |= ptcb->OSTCBBitY;                     /* Make task ready to run   */
            OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
            OS_EXIT_CRITICAL();
            OS_Sched();
        } else {
            OS_EXIT_CRITICAL();
        }
        return (OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_NOT_SUSPENDED);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                             STACK CHECKING
*
* Description: This function is called to check the amount of free memory left on the specified task's
*              stack.
*
* Arguments  : prio     is the task priority
*
*              pdata    is a pointer to a data structure of type OS_STK_DATA.
*
* Returns    : OS_NO_ERR           upon success
*              OS_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                  (i.e. > OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_TASK_NOT_EXIST   if the desired task has not been created
*              OS_TASK_OPT_ERR     if you did NOT specified OS_TASK_OPT_STK_CHK when the task was created
*********************************************************************************************************
*/
#if OS_TASK_CREATE_EXT_EN > 0
INT8U  OSTaskStkChk (INT8U prio, OS_STK_DATA *pdata)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    OS_TCB    *ptcb;
    OS_STK    *pchk;
    INT32U     free;
    INT32U     size;


#if OS_ARG_CHK_EN > 0
    if (prio > OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {        /* Make sure task priority is valid    */
        return (OS_PRIO_INVALID);
    }
#endif
    pdata->OSFree = 0;                                          /* Assume failure, set to 0 size       */
    pdata->OSUsed = 0;
    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                        /* See if check for SELF                        */
        prio = OSTCBCur->OSTCBPrio;
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb == (OS_TCB *)0) {                         /* Make sure task exist                         */
        OS_EXIT_CRITICAL();
        return (OS_TASK_NOT_EXIST);
    }
    if ((ptcb->OSTCBOpt & OS_TASK_OPT_STK_CHK) == 0) { /* Make sure stack checking option is set       */
        OS_EXIT_CRITICAL();
        return (OS_TASK_OPT_ERR);
    }
    free = 0;
    size = ptcb->OSTCBStkSize;
    pchk = ptcb->OSTCBStkBottom;
    OS_EXIT_CRITICAL();
#if OS_STK_GROWTH == 1
    while (*pchk++ == (OS_STK)0) {                    /* Compute the number of zero entries on the stk */
        free++;
    }
#else
    while (*pchk-- == (OS_STK)0) {
        free++;
    }
#endif
    pdata->OSFree = free * sizeof(OS_STK);            /* Compute number of free bytes on the stack     */
    pdata->OSUsed = (size - free) * sizeof(OS_STK);   /* Compute number of bytes used on the stack     */
    return (OS_NO_ERR);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            SUSPEND A TASK
*
* Description: This function is called to suspend a task.  The task can be the calling task if the
*              priority passed to OSTaskSuspend() is the priority of the calling task or OS_PRIO_SELF.
*
* Arguments  : prio     is the priority of the task to suspend.  If you specify OS_PRIO_SELF, the
*                       calling task will suspend itself and rescheduling will occur.
*
* Returns    : OS_NO_ERR                if the requested task is suspended
*              OS_TASK_SUSPEND_IDLE     if you attempted to suspend the idle task which is not allowed.
*              OS_PRIO_INVALID          if the priority you specify is higher that the maximum allowed
*                                       (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_TASK_SUSPEND_PRIO     if the task to suspend does not exist
*
* Note       : You should use this function with great care.  If you suspend a task that is waiting for
*              an event (i.e. a message, a semaphore, a queue ...) you will prevent this task from
*              running when the event arrives.
*********************************************************************************************************
*/

#if OS_TASK_SUSPEND_EN > 0
INT8U  OSTaskSuspend (INT8U prio)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    BOOLEAN    self;
    OS_TCB    *ptcb;


#if OS_ARG_CHK_EN > 0
    if (prio == OS_IDLE_PRIO) {                                 /* Not allowed to suspend idle task    */
        return (OS_TASK_SUSPEND_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {       /* Task priority valid ?               */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                                 /* See if suspend SELF                 */
        prio = OSTCBCur->OSTCBPrio;
        self = TRUE;
    } else if (prio == OSTCBCur->OSTCBPrio) {                   /* See if suspending self              */
        self = TRUE;
    } else {
        self = FALSE;                                           /* No suspending another task          */
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb == (OS_TCB *)0) {                                  /* Task to suspend must exist          */
        OS_EXIT_CRITICAL();
        return (OS_TASK_SUSPEND_PRIO);
    }
    if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) { /* Make task not ready                 */
        OSRdyGrp &= ~ptcb->OSTCBBitY;
    }
    ptcb->OSTCBStat |= OS_STAT_SUSPEND;                         /* Status of task is 'SUSPENDED'       */
    OS_EXIT_CRITICAL();
    if (self == TRUE) {                                         /* Context switch only if SELF         */
        OS_Sched();
    }
    return (OS_NO_ERR);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            QUERY A TASK
*
* Description: This function is called to obtain a copy of the desired task's TCB.
*
* Arguments  : prio     is the priority of the task to obtain information from.
*
* Returns    : OS_NO_ERR       if the requested task is suspended
*              OS_PRIO_INVALID if the priority you specify is higher that the maximum allowed
*                              (i.e. > OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_PRIO_ERR     if the desired task has not been created
*********************************************************************************************************
*/

#if OS_TASK_QUERY_EN > 0
INT8U  OSTaskQuery (INT8U prio, OS_TCB *pdata)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    OS_TCB    *ptcb;


#if OS_ARG_CHK_EN > 0
    if (prio > OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {   /* Task priority valid ?                    */
        return (OS_PRIO_INVALID);
    }
#endif
    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                            /* See if suspend SELF                      */
        prio = OSTCBCur->OSTCBPrio;
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb == (OS_TCB *)0) {                             /* Task to query must exist                 */
        OS_EXIT_CRITICAL();
        return (OS_PRIO_ERR);
    }
    memcpy(pdata, ptcb, sizeof(OS_TCB));                   /* Copy TCB into user storage area          */
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}
#endif
