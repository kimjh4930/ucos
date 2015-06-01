#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStksched[100];
OS_STK TestTaskStk1[100];
OS_STK TestTaskStk2[100];
OS_STK TestTaskStk3[100];

OS_EVENT *RandomSem;

void printTask();
void schedTask(void *pdata);
void TestTask1(void *pdata);
void TestTask2(void *pdata);
void TestTask3(void *pdata);
void timerCallBack(void);

volatile int frame = 0;
volatile int times = 0;

int timerCall = 0;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	consoleDemoInit();

	//irqSet(IRQ_VBLANK, Vblank);

	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	irqInit();		//libnds interrupt system 초기화

	OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate()함수를 거치면 TASK READY 상태가 되어 실행할 준비를 마친다.
	OSStart();
	//현재 코드에서는 TestTask1의 상태는 TASK RUNNING
	//TestTask2의 상태는 TASK WAITING 상태.

	return 0;
}

//semaphore 나 message box를 이용해서 해야할 듯.
void schedTask(void *pdata) {
	/* 통계 Task 초기화
	 * IDLE 태스크 외에 아무 태스크도 실행되지 않은 상태에서 IDLE 카운터 값을 얼마나 증가할 수 있는지를 알아냄.
	 */
	//OSStatInit();
	//Task 생성
	OSTaskCreate(TestTask1, (void*) 0, &TestTaskStk1[99], 2);
	OSTaskCreate(TestTask2, (void*) 0, &TestTaskStk2[99], 3);
	OSTaskCreate(TestTask3, (void*) 0, &TestTaskStk3[99], 4);

	//calls the timerCallBack function 5times per second.
	//TIMER_FREQ_256 : 2초단위
	//TIMER_FREQ_1024 : 1초단위
	timerStart(0, ClockDivider_1024, TIMER_FREQ_64(5), timerCallBack);

	while (1) {

		OSTimeDly(1);
	}
}

void TestTask1(void *pdata) {
	int time=0;

	while (1) {
		puts("TestTask1 Init");
		//printf("task1 time : %d\n",time);
		for (time = 0; time < 7500000; time++) {}
		OSTimeDly(1);

		//OSTimeDly() : 원하는 시간동안 자신의 실행을 지연할 수 있음.
		//설정한 시간이 만료되면 OSTimeTick() 함수에 의해 준비상태가 된다.
	}
}

void TestTask2(void *pdata) {
	int time=0;

	while (1) {
		//printf("\nTestTask2 Init\n");
		puts("TestTask2 Init");
		//printf("task2 time : %d\n",time);
		for (time = 0; time < 7500000; time++) {}
		OSTimeDly(1);
	}

}

void TestTask3(void *pdata) {
	int time=0;

	while (1) {
		//printf("\nTestTask3 Init\n");
		puts("TestTask3 Init");
		//printf("task3 time : %d\n",time);
		for (time = 0; time < 7500000; time++){}
		OSTimeDly(1);
	}
}

void timerCallBack() {
	puts("call\n");
	OSTimeTick();
	OS_Sched();
}
