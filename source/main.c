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
int tickArray[N_TASKS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int clockticks = 0;
int totalticks = 0;
int period = 0;
//int opTimes[8] = {10000,30000,50000,80000,100000,300000,500000,1000000};
//int opTimes[8] = { 5000, 15000, 25000, 40000, 50000, 150000, 250000, 500000 };
//int opTimes[8] = { 1000, 3000, 5000, 8000, 10000, 30000, 50000, 100000 };
int opTimes[8] = { 10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000 };
//int opTimes[8] = { 10000, 80000, 10000, 80000, 10000, 80000, 10000, 80000 };

void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
void TaskStart(void *pdata);
void timerCallBack(void);
void taskPrint(void);
static void TaskStartCreateTasks(void);
int getTicks(void);

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

	//irqInit();
	//OSStatInit();
	period = 5;

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(period), timerCallBack);
	timerStart(1, ClockDivider_1024, 0, NULL);

	TaskStartCreateTasks();

	while (1) {
		clockticks = 0;
		clockticks += timerElapsed(1);
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		tickArray[3] = clockticks;
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
		tick1 = 0;
		tick2 = 0;
		tick1 = getTicks();

		times = rand() % 8;

		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tick2 = getTicks();
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
		//clockticks = 0;
		tick3 = 0;
		tick4 = 0;
		tick3 = getTicks();

		times = rand() % 8;
		//printf("task2\n");

		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tick4 = getTicks();
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

	while (1) {
		//clockticks = 0;
		tick5 = 0;
		tick6 = 0;
		tick5 = getTicks();

		times = rand() % 8;

		for (i = 0; i < opTimes[times]; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}

		tick6 = getTicks();

		tickArray[2] = tick6 - tick5;
		OSTimeDly(1);
	}
}

void taskPrint(void) {
	iprintf(
			"\x1b[3;0Htask1 : %8d \tus\ntask2 : %8d \tus\ntask3 : %8d \tus\nutilization : %3d\t%%",
			//(tickArray[0]*10000/32728),
			//(tickArray[1]*10000/32728),
			//(tickArray[2]*10000/32728));
			((tickArray[0] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			((tickArray[1] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			((tickArray[2] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			(clockticks * 100) / (32728 / period)
			);
}

void timerCallBack() {
	OSTimeTick();
	OS_Sched();
}

int getTicks() {
	return clockticks;
}
