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
#define TIMER_SPEED (BUS_CLOCK/1024)

typedef enum {
	timerState_Stop, timerState_Pause, timerState_Running
} TimerStates;

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];
OS_STK TaskStartStk[TASK_STK_SIZE];
char TaskData[N_TASKS];
OS_EVENT *RandomSem;
int value[3];
int tickArray[N_TASKS]={0,};
int clockticks = 0;
int totalticks = 0;
int opTimes[8] = {100,500,1000,5000,10000,50000,100000,500000};

void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
void TaskStart(void *pdata);
void timerCallBack(void);
void taskPrint(void);
static void TaskStartCreateTasks(void);
int getTicks(void);
void cleartickArray();

int timerCall = 0;

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	consoleDemoInit();

	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	//irqInit();		//libnds interrupt system 초기화
	//Random 숫자 세마포어
	//OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate()함수를 거치면 TASK READY 상태가 되어 실행할 준비를 마친다.

	//RandomSem = OSSemCreate(1);
	OSTaskCreate(TaskStart, (void*) 0, &TaskStartStk[TASK_STK_SIZE - 1], 0);

	OSStart();

	return 0;
}

void TaskStart(void *pdata) {
	/*
	 * 디스플레이 초기화
	 * OSTickISR 설치
	 * OSStatInit(); //통계 Task 설치
	 * TaskStartCreateTasks();	//모든 태스크 생성
	 */

	/* 통계 Task 초기화
	 * IDLE 태스크 외에 아무 태스크도 실행되지 않은 상태에서 IDLE 카운터 값을 얼마나 증가할 수 있는지를 알아냄.
	 */

	int endtick=0;

	//irqInit();
	//OSStatInit();
	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(1), timerCallBack);
	timerStart(1, ClockDivider_1024, 0, NULL);
	TaskStartCreateTasks();

	while (1) {
		clockticks = 0;
		clockticks += timerElapsed(1);
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		taskPrint();

	}

}

void TaskStartCreateTasks(void) {

	OSTaskCreate(Task1, (void*) 0, &TaskStk[1][TASK_STK_SIZE - 1], 1);
	OSTaskCreate(Task2, (void*) 0, &TaskStk[2][TASK_STK_SIZE - 1], 2);
	OSTaskCreate(Task3, (void*) 0, &TaskStk[3][TASK_STK_SIZE - 1], 3);

}

void Task1(void *pdata) {
	int tick1 = 0;
	int tick2 = 0;
	int times = 0;
	INT32U a = 1, b = 2;
	INT32U i = 0;
	INT32U j = 0;

	while (1) {
		clockticks = 0;
		tick1 = getTicks();

		times = rand() % 8;
		//iprintf("opTimes[%d] = %d\n",times, opTimes[times]);
		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tick2 = getTicks();
		//iprintf("tick in task1 \n%d, %d, %d\n", tick1, tick2, tick2 - tick1);
		//iprintf("task1 ticks : %u\n",ticks);
		tickArray[0] = tick2 - tick1;

		OSTimeDly(1);

	}
}

void Task2(void *pdata) {
	int tick3 = 0;
	int tick4 = 0;
	int times = 0;
	INT32U a = 1, b = 2;
	INT32U i = 0;

	//printf("task create\n");

	while (1) {
		clockticks = 0;
		tick3 = getTicks();

		times = rand() % 8;
		//printf("task2\n");

		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tick4 = getTicks();
		//printf("tick in task2 \n%d, %d, %d\n", tick3, tick4, tick4 - tick3);
		//printf("task3 ticks : %u\n",getTicks());
		tickArray[1] = tick4 - tick3;
		OSTimeDly(1);

	}
}

void Task3(void *pdata) {
	INT8U err;
	INT32U i = 0;
	int tick5 = 0;
	int tick6 = 0;
	int times = 0;
	INT32U a = 1, b = 2;

	//printf("task create\n");

	while (1) {
		clockticks = 0;
		tick5 = getTicks();

		//printf("task3\n");
		times = rand() % 8;

		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}

		tick6 = getTicks();

		//iprintf("tick in task3 \n%d, %d, %d\n", tick5, tick6, tick6 - tick5);
		//printf("task1 ticks : %u\n",ticks);
		tickArray[2] = tick6 - tick5;
		OSTimeDly(1);
	}
}

void taskPrint(void) {
	iprintf("task1 : %d\ntask2 : %d\ntask3 : %d\n", tickArray[0], tickArray[1], tickArray[2]);
	//OSCPUUsage
}

void timerCallBack() {
	//taskPrint();
	//cleartickArray();
	OSTimeTick();
	OS_Sched();
}

int getTicks() {
	return clockticks;
}

void cleartickArray(){
	int i=0;

	for(i=0; i<N_TASKS; i++){
		tickArray[i]=0;
	}
}
