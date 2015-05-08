#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStk1[100];
OS_STK TestTaskStk2[100];
OS_STK TestTaskStk3[100];

void TestTask 	(void *pdata);
void TestTask2 	(void *pdata);
void TestTask3 	(void *pdata);

volatile int frame = 0;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	//irqInit();	//���ͷ�Ʈ ���� ��ġ

	consoleDemoInit();
	irqSet(IRQ_VBLANK, Vblank);
	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	OSTaskCreate(TestTask, (void*)0, &TestTaskStk1[99],1);
	OSTaskCreate(TestTask2, (void*)0, &TestTaskStk2[99],2);
	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.

	//iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();
	//���� �ڵ忡���� TestTask1�� ���´� TASK RUNNING
	//TestTask2�� ���´� TASK WAITING ����.

	return 0;
}

void TestTask (void *pdata){
	//pdata = pdata;

	//printf("TestTask1 Init before while\n");

	//timerStart(1, ClockDivider_1024, 20, (void *)OSTickISR);

	while(1){
		printf("\nTestTask1 Init\n");
		//OSTaskDel(OS_PRIO_SELF);
		OSTimeDly(5);
		//OSTimeDly() : ���ϴ� �ð����� �ڽ��� ������ ������ �� ����.
		//������ �ð��� ����Ǹ� OSTimeTick() �Լ��� ���� �غ���°� �ȴ�.
	}

}

void TestTask2 (void *pdata){

	//printf("TestTask2 Init before while\n");

	while(1){
		printf("\nTestTask2 Init\n");
		OSTimeDly(10);
	}
}


//void TIMER_IR_CLEAR(void){
//	printf("Clear init\n");
//	irqClear(IRQ_TIMER3);
//}

//void TIMER_IR_ENABLE(void){
//	printf("enable init\n");
//	irqEnable(IRQ_VCOUNT);
//}

//void CONFIRM(void){
//	confirm++;
//}
