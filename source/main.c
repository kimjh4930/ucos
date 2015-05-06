#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStk[100];

void TestTask 	(void *pdata);
void TestTask2 	(void *pdata);

volatile int frame = 0;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	irqInit();	//인터럽트 벡터 설치

	consoleDemoInit();
	irqSet(IRQ_VBLANK, Vblank);
	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");
	OSTaskCreate(TestTask, (void*)0, &TestTaskStk[99],0);
	//OSTaskCreate(TestTask2, (void*)0, &TestTaskStk[99],0);
	//iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();

	return 0;
}

void TestTask (void *pdata){
	//pdata = pdata;

	printf("TestTask1 Init before while\n");

	timerStart(1, ClockDivider_1024, 20, (void *)OSTickISR);

	while(1){
		printf("TestTask1 Init\n");
		OSTimeDly(5);
	}

}

void TestTask2 (void *pdata){

	printf("TestTask2 Init before while\n");

	while(1){
		printf("TestTask2 Init\n");
		OSTimeDly(10);
	}
}

void TIMER_IR_CLEAR(void){
	printf("Clear init\n");
	irqClear(IRQ_TIMER3);
}

void TIMER_IR_ENABLE(void){
	printf("enable init\n");
	irqEnable(IRQ_VCOUNT);
}

void CONFIRM(void){
	confirm++;
}
