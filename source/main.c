#include <nds.h>
#include <stdio.h>
#include "includes.h"

#define TASK_STK_SIZE		512
#define N_TASKS				 10

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];

void  Task(void *data);                       /* Function prototypes of tasks                  */
void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);


volatile int frame = 0;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	//touchPosition touchXY;
	//irqSet(IRQ_VBLANK, Vblank);
	consoleClear();

	OSInit();
	iprintf("\x1b[10;0H OSInit() init");
	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1],0);
	iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();
	iprintf("\x1b[10;0H OSStart() init");

	consoleDemoInit();

	while(1) {
		TaskStartDispInit();
	}

	return 0;
}

void  TaskStart (void *pdata)
{
//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
//#endif
    char       s[100];
    INT16S     key;


   // pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Initialize the display                   */

    //OS_ENTER_CRITICAL();
    //PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    //PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    //OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    TaskStartCreateTasks();                                /* Create all the application tasks         */

    for (;;) {
        TaskStartDisp();                                  /* Update the display                       */


        //if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
        //    if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
        //        PC_DOSReturn();                            /* Return to DOS                            */
        //    }
        //}

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
       // OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}

static void TaskStartDispInit (void){
	iprintf("\x1b[10;0HHello ucos world!");
}

static  void  TaskStartCreateTasks (void)
{
    INT8U  i;


    for (i = 0; i < N_TASKS; i++) {                        /* Create N_TASKS identical tasks           */
        TaskData[i] = '0' + i;                             /* Each task will display its own letter    */
        OSTaskCreate(Task, (void *)&TaskData[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
}
static  void  TaskStartDisp(void){
	iprintf("\x1b[10;0HHello ucos world!");
}

void  Task (void *pdata)
{
    INT8U  x;
    INT8U  y;
    INT8U  err;


    for (;;) {
        //OSSemPend(RandomSem, 0, &err);           /* Acquire semaphore to perform random numbers        */
        x = rand()%80;                          /* Find X position where task number will appear      */
        y = rand()%16;                          /* Find Y position where task number will appear      */
        //OSSemPost(RandomSem);                    /* Release semaphore                                  */
                                                 /* Display the task number on the screen              */
        //PC_DispChar(x, y + 5, *(char *)pdata, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        //OSTimeDly(1);                            /* Delay 1 clock tick                                 */
    }
}
