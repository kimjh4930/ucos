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

	irqInit();		//libnds interrupt system �ʱ�ȭ

	OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.
	OSStart();
	//���� �ڵ忡���� TestTask1�� ���´� TASK RUNNING
	//TestTask2�� ���´� TASK WAITING ����.

	return 0;
}

//semaphore �� message box�� �̿��ؼ� �ؾ��� ��.
void schedTask(void *pdata) {
	/* ��� Task �ʱ�ȭ
	 * IDLE �½�ũ �ܿ� �ƹ� �½�ũ�� ������� ���� ���¿��� IDLE ī���� ���� �󸶳� ������ �� �ִ����� �˾Ƴ�.
	 */
	//OSStatInit();
	//Task ����
	OSTaskCreate(TestTask1, (void*) 0, &TestTaskStk1[99], 2);
	OSTaskCreate(TestTask2, (void*) 0, &TestTaskStk2[99], 3);
	OSTaskCreate(TestTask3, (void*) 0, &TestTaskStk3[99], 4);

	//calls the timerCallBack function 5times per second.
	//TIMER_FREQ_256 : 2�ʴ���
	//TIMER_FREQ_1024 : 1�ʴ���
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

		//OSTimeDly() : ���ϴ� �ð����� �ڽ��� ������ ������ �� ����.
		//������ �ð��� ����Ǹ� OSTimeTick() �Լ��� ���� �غ���°� �ȴ�.
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
