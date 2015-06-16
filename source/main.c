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
int number[4] = {0,0,0,0};

int number1 = 0;
int number2 = 0;
int number3 = 0;
int number4 = 0;

volatile int task3interrupt = 0;

void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
void TaskStart(void *pdata);
void timerCallBack(void);
void taskPrint(void);
static void TaskStartCreateTasks(void);
int getTicks(void);
void saveReg2Stack(void);
void loadStack2Reg(void);

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
	period = 1;

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(period), timerCallBack);
	timerStart(1, ClockDivider_1024, 0, NULL);

	TaskStartCreateTasks();

	while (1) {
		clockticks = 0;
		clockticks += timerElapsed(1);
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		//tickArray[3] = clockticks;
		//taskPrint();

	}

}

void TaskStartCreateTasks(void) {

	OSTaskCreate(Task1, (void*) 0, &TaskStk[1][TASK_STK_SIZE - 1], 1);
	OSTaskCreate(Task2, (void*) 0, &TaskStk[2][TASK_STK_SIZE - 1], 2);
	OSTaskCreate(Task3, (void*) 0, &TaskStk[3][TASK_STK_SIZE - 1], 3);

}

void Task1(void *pdata) {

	INT32U a = 1, b = 2;
	INT32U i = 0;

	while (1) {
		printf("task1 init\n");
		tickArray[0] = getTicks();

		for (i = 0; i < 5000; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tickArray[1] = getTicks();
		tickArray[2] = tickArray[1] - tickArray[0];

		number1++;

		OSTimeDly(1);

	}
}

void Task2(void *pdata) {

	INT32U a = 1, b = 2;
	INT32U i = 0;

	while (1) {
		printf("task2 init\n");

		tickArray[3] = getTicks();

		for (i = 0; i < 5000; i++) {
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tickArray[4] = getTicks();
		tickArray[5] = tickArray[4] - tickArray[3];

		number2++;

		OSTimeDly(1);

	}
}

void Task3(void *pdata) {
	INT32U a = 1, b = 2;
	INT32U i = 0;

	while (1) {
		printf("task3 init\n");
		tickArray[6] = getTicks();
		for (i = 0; i < 5000; i++) {
			/*if(task3interrupt > 0){
				//printf("init\n");
				loadStack2Reg();
				task3interrupt--;
			}*/
			a = a * b;
			a = a / b;
			clockticks += timerElapsed(1);
		}
		tickArray[7] = getTicks();
		tickArray[8] = tickArray[7] - tickArray[6];

		number3++;
		OSTimeDly(1);
	}
}

void taskPrint(void) {
	//period : 32728/period * 1000 ms
	/*iprintf(
			"\x1b[3;0Htask1 : %8d \tus\ntask2 : %8d \tus\ntask3 : %8d \tus\nutilization : %3d\t%%",
			//(tickArray[0]*10000/32728),
			//(tickArray[1]*10000/32728),
			//(tickArray[2]*10000/32728));
			((tickArray[0] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			((tickArray[1] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			((tickArray[2] % TIMER_SPEED) * 1000000) / TIMER_SPEED,
			(clockticks * 100) / (32728 / period)
			);*/
	/*iprintf("\x1b[3;0Htask1 init : %8d\tms\ntask1 exit : %8d\tms\ntask1 util : %8d\t%%\n"
			"task2 init : %8d\tms\ntask2 exit : %8d\tms\ntask2 util : %8d\t%%\n"
			"task3 init : %8d\tms\ntask3 exit : %8d\tms\ntask3 util : %8d\t%%\n"
			"time : %8d\ttimes",
			tickArray[0], tickArray[1], ((tickArray[2]*100)/(32728/period)),
			tickArray[3], tickArray[4], ((tickArray[5]*100)/(32728/period)),
			tickArray[6], tickArray[7], ((tickArray[8]*100)/(32728/period)),
			number);*/
	iprintf("\x1b[3;0H1 init : %5d exit : %5d\ntime : %d\n"
			"2 init : %5d exit : %5d\ntime : %d\n"
			"3 init : %5d exit : %5d\ntime : %d\n"
			"total time : %d",
			tickArray[0], tickArray[1], number1,
			tickArray[3], tickArray[4], number2,
			tickArray[6], tickArray[7], number3,
			number4);

}

void timerCallBack() {			//OSTickISR 과 같은 역할을 하도록 수정?
	//현재 태스크의 모든 문맥을 스택에 저장
	printf("called timerCallBack\n");

	saveReg2Stack();
	OSIntNesting++;

	//printf("1\n");

	if(OSIntNesting == 1){
		__asm__ __volatile__("LDR     R4, =OSTCBCur");
		__asm__ __volatile__("LDR     R5, [R4]");
		__asm__ __volatile__("STR     SP, [R5]");
	}
	//printf("2\n");

	number4++;
	OSTimeTick();
	OSIntExit();
	OS_Sched();


	//loadStack2Reg();


}

int getTicks() {
	return clockticks;
}

void saveReg2Stack(void){

	__asm__ __volatile__("STR	LR,  [SP, #-4]!");
	__asm__ __volatile__("STR	LR,  [SP, #-4]!");
	__asm__ __volatile__("STR	R12, [SP, #-4]!");
	__asm__ __volatile__("STR	R11, [SP, #-4]!");
	__asm__ __volatile__("STR	R10, [SP, #-4]!");
	__asm__ __volatile__("STR	R9,  [SP, #-4]!");
	__asm__ __volatile__("STR	R8,  [SP, #-4]!");
	__asm__ __volatile__("STR	R7,  [SP, #-4]!");
	__asm__ __volatile__("STR	R6,  [SP, #-4]!");
	__asm__ __volatile__("STR	R5,  [SP, #-4]!");
	__asm__ __volatile__("STR	R4,  [SP, #-4]!");
	__asm__ __volatile__("STR	R3,  [SP, #-4]!");
	__asm__ __volatile__("STR	R2,  [SP, #-4]!");
	__asm__ __volatile__("STR	R1,  [SP, #-4]!");
	__asm__ __volatile__("STR	R0,  [SP, #-4]!");
	__asm__ __volatile__("MRS	R4,  CPSR");
	__asm__ __volatile__("STR	R4,  [SP, #-4]!");
}

void loadStack2Reg(void){

	__asm__ __volatile__("LDR	R4,  [SP], #4");
	__asm__ __volatile__("MSR	CPSR_cxsf, R4");
	__asm__ __volatile__("LDR	R0,  [SP], #4");
	__asm__ __volatile__("LDR	R1,  [SP], #4");
	__asm__ __volatile__("LDR	R2,  [SP], #4");
	__asm__ __volatile__("LDR	R3,  [SP], #4");
	__asm__ __volatile__("LDR	R4,  [SP], #4");
	__asm__ __volatile__("LDR	R5,  [SP], #4");
	__asm__ __volatile__("LDR	R6,  [SP], #4");
	__asm__ __volatile__("LDR	R7,  [SP], #4");
	__asm__ __volatile__("LDR	R8,  [SP], #4");
	__asm__ __volatile__("LDR	R9,  [SP], #4");
	__asm__ __volatile__("LDR	R10, [SP], #4");
	__asm__ __volatile__("LDR	R11, [SP], #4");
	__asm__ __volatile__("LDR	R12, [SP], #4");
	__asm__ __volatile__("LDR	LR,  [SP], #4");
	__asm__ __volatile__("LDR	PC,  [SP], #4");
}
