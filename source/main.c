#include <nds.h>
#include <stdio.h>
#include <time.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

#define TASK_STK_SIZE		100
#define N_TASKS				 10

OS_STK	TaskStk[N_TASKS][TASK_STK_SIZE];
OS_STK	TaskStartStk[TASK_STK_SIZE];
char	TaskData[N_TASKS];
OS_EVENT *RandomSem;
int		value[5];

		void Task1(void *pdata);
		void Task2(void *pdata);
		void Task3(void *pdata);
		void TaskStart(void *pdata);
		void timerCallBack(void);
		void taskPrint(void);
static 	void TaskStartCreateTasks(void);

int timerCall = 0;

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	consoleDemoInit();

	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	//irqInit();		//libnds interrupt system �ʱ�ȭ
	//Random ���� ��������
	//OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.

	RandomSem = OSSemCreate(1);
	OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE-1], 0);
	OSStart();

	return 0;
}

void TaskStart(void *pdata){

	uint	ticks = 0;
	uint	i = 0;
	uint 	tick1=0, tick2=0;
	/*
	 * ���÷��� �ʱ�ȭ
	 * OSTickISR ��ġ
	 * OSStatInit(); //��� Task ��ġ
	 * TaskStartCreateTasks();	//��� �½�ũ ����
	 */

	/* ��� Task �ʱ�ȭ
	 * IDLE �½�ũ �ܿ� �ƹ� �½�ũ�� ������� ���� ���¿��� IDLE ī���� ���� �󸶳� ������ �� �ִ����� �˾Ƴ�.
	 */

	irqInit();
	//OSStatInit();
	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(2), timerCallBack);
	//timerStart(1, ClockDivider_1024, 0, NULL);
	timerStart(2, ClockDivider_1024, 0, NULL);

	TaskStartCreateTasks();

	while(1){
		ticks = 0;
		ticks += timerElapsed(2);
		tick1 = ticks;
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		tick2 = ticks;
		printf("start : 1=%u, 2:%u, 2-1:%u\n",tick1, tick2, (int)tick2-(int)tick1);
		//printf("ticks = %u\n",ticks);
		//OSTimeDlyHMSM(0,0,0,200);
		//taskPrint();
	}

}

void TaskStartCreateTasks(void){

	OSTaskCreate(Task1, (void*)0, &TaskStk[1][TASK_STK_SIZE-1], 1);
	OSTaskCreate(Task2, (void*)0, &TaskStk[2][TASK_STK_SIZE-1], 2);
	OSTaskCreate(Task3, (void*)0, &TaskStk[3][TASK_STK_SIZE-1], 3);

}

void Task1(void *pdata){
	INT8U err;
	uint ticks;
	uint tick1=0, tick2=0;

	//printf("task create\n");
	timerStart(1, ClockDivider_1024, 0, NULL);
	while(1){
		timerUnpause(1);
		ticks = 0;

		//OSSemPend(RandomSem, 0, &err);
		ticks += timerElapsed(1);
		//value[0] = rand()%10+1;
		//OSSemPost(RandomSem);
		//tick1 = ticks;
		OSTimeDly(1);
		ticks += timerPause(1);
		//tick2 = ticks;
		//printf("task1 : 1=%u, 2:%u, 2-1:%u\n",tick1, tick2, (int)tick2-(int)tick1);
		//OSTimeDlyHMSM(0,0,0,200);
		//printf("task1 execute\n");
	}
}

void Task2(void *pdata){
	INT8U err;
	INT32U	i;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[1] = rand()%10+1;
		OSSemPost(RandomSem);
		for(i=0; i<1000000; i++){}
		OSTimeDly(1);
		//OSTimeDlyHMSM(0,0,0,200);
		printf("task2 execute\n");
	}
}

void Task3(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[2] = rand()%10+1;
		OSSemPost(RandomSem);
		OSTimeDly(1);
		//OSTimeDlyHMSM(0,0,0,200);
		printf("task3 execute\n");
	}
}

void taskPrint(void){
	INT8U index;

	printf("value[0] = %d\n"
			"value[1] = %d\n"
			"value[2] = %d\n"
			"value[3] = %d\n"
			"value[4] = %d\n",value[0],value[1],value[2],value[3],value[4]);
	//OSCPUUsage
}



void timerCallBack() {
	puts("call\n");
	OSTimeTick();
	OS_Sched();
}
