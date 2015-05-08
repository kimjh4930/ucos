#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStk1[100];
OS_STK TestTaskStk2[100];

void TestTask 	(void *pdata);
void TestTask2 	(void *pdata);

INT8U	prio1 = 0;
INT8U	prio2 = 1;

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

	OSTaskCreate(TestTask, (void*)0, &TestTaskStk1[99],prio1);
	OSTaskCreate(TestTask2, (void*)0, &TestTaskStk2[99],prio2);

	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.

	//iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();
	//���� �ڵ忡���� TestTask1�� ���´� TASK RUNNING
	//TestTask2�� ���´� TASK WAITING ����.

	return 0;
}

void TestTask (void *pdata){

	INT8U temp;
	//pdata = pdata;

	//printf("TestTask1 Init before while\n");

	//timerStart(1, ClockDivider_1024, 20, (void *)OSTickISR);

	while(1){
		printf("\nTestTask1 Init\n");
		//OSTaskDel(OS_PRIO_SELF);
		temp = prio1+2;
		OSTaskChangePrio(prio1,temp);
		//printf("prio1 : %d\n",prio1);
		OSTimeDly(5);
		//OSTimeDly() : ���ϴ� �ð����� �ڽ��� ������ ������ �� ����.
		//������ �ð��� ����Ǹ� OSTimeTick() �Լ��� ���� �غ���°� �ȴ�.
	}

}

void TestTask2 (void *pdata){

	//printf("TestTask2 Init before while\n");

	INT8U temp = 0;

	while(1){
		printf("\nTestTask2 Init\n");
		//temp = prio2+2;
		//OSTaskChangePrio(prio2,temp);
		//printf("prio2 : %d\n",prio2);
		OSTimeDly(5);
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
