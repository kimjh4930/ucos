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
void timerCallBack (void);

INT8U	prio1 = 0;
INT8U	prio2 = 1;

int channel = 0;
bool play = true;

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
	soundEnable();
	//irqInit();		//libnds interrupt system �ʱ�ȭ

	channel = soundPlayPSG(DutyCycle_50, 10000, 127, 64);

	irqSet(IRQ_VBLANK, Vblank);

	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	OSTaskCreate(TestTask, (void*)0, &TestTaskStk1[99],1);
	OSTaskCreate(TestTask2, (void*)0, &TestTaskStk2[99],2);
	//OSTaskCreate()�Լ��� ��ġ�� TASK READY ���°� �Ǿ� ������ �غ� ��ģ��.
	OSStart();
	//���� �ڵ忡���� TestTask1�� ���´� TASK RUNNING
	//TestTask2�� ���´� TASK WAITING ����.

	return 0;
}

void TestTask (void *pdata){

	//INT8U temp;
	//pdata = pdata;

	//printf("TestTask1 Init before while\n");

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(5), timerCallBack);
	//timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(5), (void *)OSTickISR);
	printf("timerStart\n");

	while(1){
		printf("\nTestTask1 Init\n");
		OSTimeDly(10);
		//OSTimeDly() : ���ϴ� �ð����� �ڽ��� ������ ������ �� ����.
		//������ �ð��� ����Ǹ� OSTimeTick() �Լ��� ���� �غ���°� �ȴ�.
	}

}

void TestTask2 (void *pdata){

	//printf("TestTask2 Init before while\n");

	//INT8U temp = 0;

	while(1){
		printf("\nTestTask2 Init\n");
		OSTimeDly(10);
	}
}

void timerCallBack(){
	if(play){
		soundPause(channel);
	}else{
		soundPause(channel);
	}

	play = !play;
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
