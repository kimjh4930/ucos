/*
                                               uC/OS-II
                                         The Real-Time Kernel
                               (c) Copyright 1992-2004, Micrium, Weston, FL
                                          All Rights Reserved
                                           ARM Generic Port
                                            IAR C Compiler
 File    : OS_CPU_A.ASM
 Version : V1.50
 By      : Jean J. Labrosse
*/

            .extern  	OSRunning                    // External references
            .extern  	OSPrioCur
            .extern  	OSPrioHighRdy
            .extern  	OSTCBCur
            .extern  	OSTCBHighRdy
            .extern  	OSIntNesting
            .extern  	OSIntExit
            .extern  	OSTaskSwHook
            .extern  	OS_CPU_IRQ_ISR_Handler
            .extern  	OS_CPU_FIQ_ISR_Handler

            //.extern	 	TIMER_IR_CLEAR
            //.extern		TIMER_IR_ENABLE
            //.extern		OSIntExit
            .extern		CONFIRM


            .globl  	OS_CPU_SR_Save               // Functions declared in this file
            .globl  	OS_CPU_SR_Restore
            .globl 		OSStartHighRdy
            .globl  	OSCtxSw
            .globl  	OSIntCtxSw
            .globl  	OS_CPU_IRQ_ISR
            .globl  	OS_CPU_FIQ_ISR

            //.globl		OSTickISR


			.equ	NO_INT,           0xC0                         // Mask used to disable interrupts (Both FIR and IRQ)
			.equ	SYS32_MODE,       0x1F
			.equ	FIQ32_MODE,       0x11
			.equ	IRQ32_MODE,       0x12



//                                   CRITICAL SECTION METHOD 3 FUNCTIONS
//
// Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
//              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
//              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
//              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
//              into the CPU's status register.
//
// Prototypes :     OS_CPU_SR  OS_CPU_SR_Save(void);
//                  void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
//
//
// Note(s)    : 1) These functions are used in general like this:
//
//                 void Task (void *p_arg)
//                 {
//                 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
//                     OS_CPU_SR  cpu_sr;
//                 #endif
//
//                          :
//                          :
//                     OS_ENTER_CRITICAL();             /* cpu_sr = OS_CPU_SaveSR();                */
//                          :
//                          :
//                     OS_EXIT_CRITICAL();              /* OS_CPU_RestoreSR(cpu_sr);                */
//                          :
//                          :
//                 }
//
//              2) OS_CPU_SaveSR() is implemented as recommended by Atmel's application note:
//
//                    "Disabling Interrupts at Processor Level"
       // RSEG CODE:CODE:NOROOT(2)
       // CODE32
       .code 32

OS_CPU_SR_Save:
        MRS     R0,CPSR                     // Set IRQ and FIQ bits in CPSR to disable all interrupts
        ORR     R1,R0,#NO_INT
        MSR     CPSR_c,R1
        MRS     R1,CPSR                     // Confirm that CPSR contains the proper interrupt disable flags
        AND     R1,R1,#NO_INT
        CMP     R1,#NO_INT
        BNE     OS_CPU_SR_Save              // Not properly disabled (try again)
        MOV     PC,LR                       // Disabled, return the original CPSR contents in R0
OS_CPU_SR_Restore:
        MSR     CPSR_c,R0
        MOV     PC,LR
/*
                                          START MULTITASKING
                                       void OSStartHighRdy(void)
 Note(s) : 1) OSStartHighRdy() MUST:
              a) Call OSTaskSwHook() then,
              b) Set OSRunning to TRUE,
              c) Switch to the highest priority task.
*/
        //RSEG CODE:CODE:NOROOT(2)
        //CODE32

        .code 32

OSStartHighRdy:

        MSR     CPSR_cxsf, #0xDF        // Switch to SYS mode with IRQ and FIQ disabled
        BL      =OSTaskSwHook            // OSTaskSwHook();
        LDR     R4, =OSRunning           // OSRunning = TRUE
        MOV     R5, #1
        STRB    R5, [R4]
                                        // SWITCH TO HIGHEST PRIORITY TASK
        LDR     R4, =OSTCBHighRdy       //    Get highest priority task TCB address
        LDR     R4, [R4]                //    get stack pointer
        LDR     SP, [R4]                //    switch to the new stack
        LDR     R4,  [SP], #4           //    pop new task's CPSR
        MSR     CPSR_cxsf,R4
        LDR     R0,  [SP], #4           //    pop new task's context
        LDR     R1,  [SP], #4
        LDR     R2,  [SP], #4
        LDR     R3,  [SP], #4
        LDR     R4,  [SP], #4
        LDR     R5,  [SP], #4
        LDR     R6,  [SP], #4
        LDR     R7,  [SP], #4
        LDR     R8,  [SP], #4
        LDR     R9,  [SP], #4
        LDR     R10, [SP], #4
        LDR     R11, [SP], #4
        LDR     R12, [SP], #4
        LDR     LR,  [SP], #4
        LDR     PC,  [SP], #4

/*
                         PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
 Note(s) : 1) OSCtxSw() is called in SYS mode with BOTH FIQ and IRQ interrupts DISABLED
           2) The pseudo-code for OSCtxSw() is:
              a) Save the current task's context onto the current task's stack
              b) OSTCBCur->OSTCBStkPtr = SP;
              c) OSTaskSwHook();
              d) OSPrioCur             = OSPrioHighRdy;
              e) OSTCBCur              = OSTCBHighRdy;
              f) SP                    = OSTCBHighRdy->OSTCBStkPtr;
              g) Restore the new task's context from the new task's stack
              h) Return to new task's code
           3) Upon entry:
              OSTCBCur      points to the OS_TCB of the task to suspend
              OSTCBHighRdy  points to the OS_TCB of the task to resume
*/

        //RSEG CODE:CODE:NOROOT(2)
        //CODE32

        .code 32

OSCtxSw:
                                        // SAVE CURRENT TASK'S CONTEXT
        STR     LR,  [SP, #-4]!         //     Return address
        STR     LR,  [SP, #-4]!
        STR     R12, [SP, #-4]!
        STR     R11, [SP, #-4]!
        STR     R10, [SP, #-4]!
        STR     R9,  [SP, #-4]!
        STR     R8,  [SP, #-4]!
        STR     R7,  [SP, #-4]!
        STR     R6,  [SP, #-4]!
        STR     R5,  [SP, #-4]!
        STR     R4,  [SP, #-4]!
        STR     R3,  [SP, #-4]!
        STR     R2,  [SP, #-4]!
        STR     R1,  [SP, #-4]!
        STR     R0,  [SP, #-4]!
        MRS     R4,  CPSR               //    push current CPSR
        STR     R4,  [SP, #-4]!

        LDR     R4, =OSTCBCur         // OSTCBCur->OSTCBStkPtr = SP;
        LDR     R5, [R4]
        STR     SP, [R5]
        BL      =OSTaskSwHook            // OSTaskSwHook();
        LDR     R4, =OSPrioCur        // OSPrioCur = OSPrioHighRdy
        LDR     R5, =OSPrioHighRdy
        LDRB    R6, [R5]
        STRB    R6, [R4]

        LDR     R4, =OSTCBCur         // OSTCBCur  = OSTCBHighRdy;
        LDR     R6, =OSTCBHighRdy
        LDR     R6, [R6]
        STR     R6, [R4]
        LDR     SP, [R6]                // SP = OSTCBHighRdy->OSTCBStkPtr;
                                        // RESTORE NEW TASK'S CONTEXT
        LDR     R4,  [SP], #4           //    pop new task's CPSR
        MSR     CPSR_cxsf, R4
        LDR     R0,  [SP], #4           //    pop new task's context
        LDR     R1,  [SP], #4
        LDR     R2,  [SP], #4
        LDR     R3,  [SP], #4
        LDR     R4,  [SP], #4
        LDR     R5,  [SP], #4
        LDR     R6,  [SP], #4
        LDR     R7,  [SP], #4
        LDR     R8,  [SP], #4
        LDR     R9,  [SP], #4
        LDR     R10, [SP], #4
        LDR     R11, [SP], #4
        LDR     R12, [SP], #4
        LDR     LR,  [SP], #4
        LDR     PC,  [SP], #4


/*
                   PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
 Note(s) : 1) OSIntCtxSw() is called in SYS mode with BOTH FIQ and IRQ interrupts DISABLED
           2) The pseudo-code for OSCtxSw() is:
              a) OSTaskSwHook();
              b) OSPrioCur             = OSPrioHighRdy;
              c) OSTCBCur              = OSTCBHighRdy;
              d) SP                    = OSTCBHighRdy->OSTCBStkPtr;
              e) Restore the new task's context from the new task's stack
              f) Return to new task's code
           3) Upon entry:
              OSTCBCur      points to the OS_TCB of the task to suspend
              OSTCBHighRdy  points to the OS_TCB of the task to resume
*/

        //RSEG CODE:CODE:NOROOT(2)
        //CODE32

        .code 32

OSIntCtxSw:
        BL      =OSTaskSwHook            // OSTaskSwHook();

        LDR     R4,=OSPrioCur         // OSPrioCur = OSPrioHighRdy
        LDR     R5,=OSPrioHighRdy
        LDRB    R6,[R5]
        STRB    R6,[R4]

        LDR     R4,=OSTCBCur          // OSTCBCur  = OSTCBHighRdy;
        LDR     R6,=OSTCBHighRdy
        LDR     R6,[R6]
        STR     R6,[R4]

        LDR     SP,[R6]                 // SP = OSTCBHighRdy->OSTCBStkPtr;

                                        // RESTORE NEW TASK'S CONTEXT
        LDR     R4,  [SP], #4           //    pop new task's CPSR
        MSR     CPSR_cxsf, R4
        LDR     R0,  [SP], #4           //    pop new task's context
        LDR     R1,  [SP], #4
        LDR     R2,  [SP], #4
        LDR     R3,  [SP], #4
        LDR     R4,  [SP], #4
        LDR     R5,  [SP], #4
        LDR     R6,  [SP], #4
        LDR     R7,  [SP], #4
        LDR     R8,  [SP], #4
        LDR     R9,  [SP], #4
        LDR     R10, [SP], #4
        LDR     R11, [SP], #4
        LDR     R12, [SP], #4
        LDR     LR,  [SP], #4
        LDR     PC,  [SP], #4


/*
                                      IRQ Interrupt Service Routine
*/

		.code 32
        //RSEG CODE:CODE:NOROOT(2)
        //CODE32

OS_CPU_IRQ_ISR:

        STR     R3,  [SP, #-4]!                // PUSH WORKING REGISTERS ONTO IRQ STACK
        STR     R2,  [SP, #-4]!
        STR     R1,  [SP, #-4]!

        MOV     R1, SP                         // Save   IRQ stack pointer

        ADD     SP, SP,#12                     // Adjust IRQ stack pointer

        SUB     R2, LR,#4                      // Adjust PC for return address to task

        MRS     R3, SPSR                       // Copy SPSR (i.e. interrupted task's CPSR) to R3

        MSR     CPSR_c, #(NO_INT | SYS32_MODE) // Change to SYS mode
                                               // SAVE TASK'S CONTEXT ONTO TASK'S STACK
        STR     R2,  [SP, #-4]!                //    Push task's Return PC
        STR     LR,  [SP, #-4]!                //    Push task's LR
        STR     R12, [SP, #-4]!                //    Push task's R12-R4
        STR     R11, [SP, #-4]!
        STR     R10, [SP, #-4]!
        STR     R9,  [SP, #-4]!
        STR     R8,  [SP, #-4]!
        STR     R7,  [SP, #-4]!
        STR     R6,  [SP, #-4]!
        STR     R5,  [SP, #-4]!
        STR     R4,  [SP, #-4]!

        LDR     R4,  [R1], #4                  //    Move task's R1-R3 from IRQ stack to SYS stack
        LDR     R5,  [R1], #4
        LDR     R6,  [R1], #4
        STR     R6,  [SP, #-4]!
        STR     R5,  [SP, #-4]!
        STR     R4,  [SP, #-4]!

        STR     R0,  [SP, #-4]!                //    Push task's R0    onto task's stack
        STR     R3,  [SP, #-4]!                //    Push task's CPSR (i.e. IRQ's SPSR)

                                               // HANDLE NESTING COUNTER
        LDR     R0, =OSIntNesting            // OSIntNesting++;
        LDRB    R1, [R0]
        ADD     R1, R1,#1
        STRB    R1, [R0]

        CMP     R1, #1                         // if (OSIntNesting == 1) {
        BNE     OS_CPU_IRQ_ISR_1

        LDR     R4, =OSTCBCur                //     OSTCBCur->OSTCBStkPtr = SP
        LDR     R5, [R4]
        STR     SP, [R5]                       // }

OS_CPU_IRQ_ISR_1:
        MSR     CPSR_c, #(NO_INT | IRQ32_MODE) // Change to IRQ mode (to use the IRQ stack to handle interrupt)

        BL      =OS_CPU_IRQ_ISR_Handler         // OS_CPU_IRQ_ISR_Handler();

        MSR     CPSR_c, #(NO_INT | SYS32_MODE) // Change to SYS mode

        BL      OSIntExit                     // OSIntExit();

                                              // RESTORE TASK'S CONTEXT and RETURN TO TASK
        LDR     R4,  [SP], #4                 //    pop new task's CPSR
        MSR     CPSR_cxsf, R4
        LDR     R0,  [SP], #4                 //    pop new task's context
        LDR     R1,  [SP], #4
        LDR     R2,  [SP], #4
        LDR     R3,  [SP], #4
        LDR     R4,  [SP], #4
        LDR     R5,  [SP], #4
        LDR     R6,  [SP], #4
        LDR     R7,  [SP], #4
        LDR     R8,  [SP], #4
        LDR     R9,  [SP], #4
        LDR     R10, [SP], #4
        LDR     R11, [SP], #4
        LDR     R12, [SP], #4
        LDR     LR,  [SP], #4
        LDR     PC,  [SP], #4


/*
                                      FIQ Interrupt Service Routine
*/

        //RSEG CODE:CODE:NOROOT(2)
        //CODE32

        .code 32

OS_CPU_FIQ_ISR:

        STMFD   SP!,{R1-R3}                   // PUSH WORKING REGISTERS ONTO IRQ STACK

        MOV     R1,SP                         // Save   IRQ stack pointer

        ADD     SP,SP,#12                     // Adjust FIQ stack pointer

        SUB     R2,LR,#4                      // Adjust PC for return address to task

        MRS     R3,SPSR                       // Copy SPSR (i.e. interrupted task's CPSR) to R3

        MSR     CPSR_c,#(NO_INT | SYS32_MODE) // Change to SYS mode
                                              // SAVE TASK'S CONTEXT ONTO TASK'S STACK
        STMFD   SP!,{R2}                      //    Push task's Return PC
        STMFD   SP!,{R4-R12,LR}               //    Push task's LR,R12-R4

        LDMFD   R1!,{R4-R6}                   //    Move task's R1-R3 from IRQ stack to SYS stack
        STMFD   SP!,{R4-R6}
        STMFD   SP!,{R0}                      //    Push task's R0    onto task's stack
        STMFD   SP!,{R3}                      //    Push task's CPSR (i.e. IRQ's SPSR)

                                              // HANDLE NESTING COUNTER
        LDR     R0,=OSIntNesting            // OSIntNesting++;
        LDRB    R1,[R0]
        ADD     R1,R1,#1
        STRB    R1,[R0]

        CMP     R1,#1                         // if (OSIntNesting == 1) {
        BNE     OS_CPU_FIQ_ISR_1

        LDR     R4,=OSTCBCur                //     OSTCBCur->OSTCBStkPtr = SP
        LDR     R5,[R4]
        STR     SP,[R5]                       // }

OS_CPU_FIQ_ISR_1:
        MSR     CPSR_c,#(NO_INT | FIQ32_MODE) // Change to FIQ mode (to use the FIQ stack to handle interrupt)

        BL      =OS_CPU_FIQ_ISR_Handler        // OS_CPU_FIQ_ISR_Handler();

        MSR     CPSR_c,#(NO_INT | SYS32_MODE) // Change to SYS mode

        BL      OSIntExit                     // OSIntExit();

                                              // RESTORE TASK'S CONTEXT and RETURN TO TASK
        LDMFD   SP!,{R4}                      // pop new task's CPSR
        MSR     CPSR_cxsf,r4
        LDMFD   SP!,{R0-R12,LR,PC}            // pop new task's R0-R12,LR & PC

OSTickISR:

		//BL		=CONFIRM
											   // 프로세서 레지스터 저장
		STR     R3,  [SP, #-4]!                // PUSH WORKING REGISTERS ONTO IRQ STACK
        STR     R2,  [SP, #-4]!
        STR     R1,  [SP, #-4]!

        MOV     R1, SP                         // Save   IRQ stack pointer

        ADD     SP, SP,#12                     // Adjust IRQ stack pointer

        SUB     R2, LR,#4                      // Adjust PC for return address to task

        MRS     R3, SPSR                       // Copy SPSR (i.e. interrupted task's CPSR) to R3

        MSR     CPSR_c, #(NO_INT | SYS32_MODE) // Change to SYS mode
                                               // SAVE TASK'S CONTEXT ONTO TASK'S STACK
        STR     R2,  [SP, #-4]!                //    Push task's Return PC
        STR     LR,  [SP, #-4]!                //    Push task's LR
        STR     R12, [SP, #-4]!                //    Push task's R12-R4
        STR     R11, [SP, #-4]!
        STR     R10, [SP, #-4]!
        STR     R9,  [SP, #-4]!
        STR     R8,  [SP, #-4]!
        STR     R7,  [SP, #-4]!
        STR     R6,  [SP, #-4]!
        STR     R5,  [SP, #-4]!
        STR     R4,  [SP, #-4]!

        LDR     R4,  [R1], #4                  //    Move task's R1-R3 from IRQ stack to SYS stack
        LDR     R5,  [R1], #4
        LDR     R6,  [R1], #4
        STR     R6,  [SP, #-4]!
        STR     R5,  [SP, #-4]!
        STR     R4,  [SP, #-4]!

        STR     R0,  [SP, #-4]!                //    Push task's R0    onto task's stack
        STR     R3,  [SP, #-4]!                //    Push task's CPSR (i.e. IRQ's SPSR)

		LDR	R0, =OSIntNesting	//OSIntNesting++;
		LDRB	R1, [R0]
		ADD	R1, R1,#1
		STRB	R1, [R0]

		CMP	R1, #1			//if(OSIntNesting == 1){
		BNE	OS_CPU_IRQ_ISR_1

		LDR	R4, =OSTCBCur		//OSTCBCur->OSTCBStkPtr = SP
		LDR	R5, [R4]
		STR	SP, [R5]		//}

		BL	=OSTimeTick			//OSTimeTick()호출
		BL	=TIMER_IRQ_CLEAR	//인터럽트 발생장치 클리어
		BL	=TIMER_IRQ_ENABLE	//인터럽트 재활성화(선택)
		BL	=OSIntExit
						//레지스터 복구
		MSR     CPSR_c, #(NO_INT | IRQ32_MODE) // Change to IRQ mode (to use the IRQ stack to handle interrupt)

        BL      =OS_CPU_IRQ_ISR_Handler         // OS_CPU_IRQ_ISR_Handler();

        MSR     CPSR_c, #(NO_INT | SYS32_MODE) // Change to SYS mode

        BL      OSIntExit                     // OSIntExit();

                                              // RESTORE TASK'S CONTEXT and RETURN TO TASK
        LDR     R4,  [SP], #4                 //    pop new task's CPSR
        MSR     CPSR_cxsf, R4
        LDR     R0,  [SP], #4                 //    pop new task's context
        LDR     R1,  [SP], #4
        LDR     R2,  [SP], #4
        LDR     R3,  [SP], #4
        LDR     R4,  [SP], #4
        LDR     R5,  [SP], #4
        LDR     R6,  [SP], #4
        LDR     R7,  [SP], #4
        LDR     R8,  [SP], #4
        LDR     R9,  [SP], #4
        LDR     R10, [SP], #4
        LDR     R11, [SP], #4
        LDR     R12, [SP], #4
        LDR     LR,  [SP], #4
        LDR     PC,  [SP], #4
