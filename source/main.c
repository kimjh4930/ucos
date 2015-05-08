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

	//irqInit();	//인터럽트 벡터 설치

	consoleDemoInit();
	irqSet(IRQ_VBLANK, Vblank);
	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	OSTaskCreate(TestTask, (void*)0, &TestTaskStk1[99],1);
	OSTaskCreate(TestTask2, (void*)0, &TestTaskStk2[99],2);
	//OSTaskCreate()함수를 거치면 TASK READY 상태가 되어 실행할 준비를 마친다.

	//iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();
	//현재 코드에서는 TestTask1의 상태는 TASK RUNNING
	//TestTask2의 상태는 TASK WAITING 상태.

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
		//OSTimeDly() : 원하는 시간동안 자신의 실행을 지연할 수 있음.
		//설정한 시간이 만료되면 OSTimeTick() 함수에 의해 준비상태가 된다.
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
