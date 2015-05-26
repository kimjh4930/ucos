#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStksched[100];
OS_STK TestTaskStkprint[100];
OS_STK TestTaskStk1[100];
OS_STK TestTaskStk2[100];
OS_STK TestTaskStk3[100];

void printTask();
void schedTask(void *pdata);
void TestTask1(void *pdata);
void TestTask2(void *pdata);
void TestTask3(void *pdata);
void timerCallBack(void);

volatile int frame = 0;
volatile int times = 0;

volatile int i = 0;
volatile int j = 0;
volatile int k = 0;

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
	//iprintf("\x1b[1;0H ----OSInit() init----\n");

	irqInit();		//libnds interrupt system �ʱ�ȭ

	irqSet(IRQ_VBLANK, printTask);
	OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate(printTask, (void*) 0, &TestTaskStkprint[99], 2);
	OSTaskCreate(TestTask1, (void*) 0, &TestTaskStk1[99], 3);
	OSTaskCreate(TestTask2, (void*) 0, &TestTaskStk2[99], 4);
	OSTaskCreate(TestTask3, (void*) 0, &TestTaskStk3[99], 5);
	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.
	OSStart();
	//���� �ڵ忡���� TestTask1�� ���´� TASK RUNNING
	//TestTask2�� ���´� TASK WAITING ����.

	return 0;
}

void schedTask(void *pdata) {
	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(5), timerCallBack);
	while (1) {
		OSTimeDly(1);
		printTask();
	}
}

void printTask() {

	while (1) {
		iprintf("\x1b[1;0Hi : %d", i);
		iprintf("\x1b[2;0Hj : %d", j);
		iprintf("\x1b[3;0Hk : %d", k);
		OSTimeDly(1);
	}
}

void TestTask1(void *pdata) {
	int time;
	//int i=0;

	while (1) {
		//printf("\nTestTask1 Init\n");
		i++;
		for (time = 0; time < 500000; time++) {
		}
		//iprintf("\x1b[1;0Hi : ");
		//printf("i : %d\n",i);
		OSTimeDly(10);
		//OSTimeDly() : ���ϴ� �ð����� �ڽ��� ������ ������ �� ����.
		//������ �ð��� ����Ǹ� OSTimeTick() �Լ��� ���� �غ���°� �ȴ�.
	}
}

void TestTask2(void *pdata) {

	int time;
	//int j=0;

	while (1) {
		//printf("\nTestTask2 Init\n");
		j++;
		for(time=0; time < 1000000; time++){
		}
		//iprintf("\x1b[1;10Hj : ");
		OSTimeDly(10);
	}

}

void TestTask3(void *pdata) {

	int time;
	//int k=0;

	while (1) {
		//printf("\nTestTask3 Init\n");
		k++;
		for(time=0; time < 1500000; time++){
		}
		//iprintf("\x1b[1;15Hk : ");
		OSTimeDly(10);
	}
}

void timerCallBack() {
	//printf("call\n");
	OSTimeTick();
	OS_Sched();
}
