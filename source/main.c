#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

OS_STK TestTaskStk[100];

void TestTask (void *pdata);

volatile int frame = 0;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	consoleDemoInit();
	irqSet(IRQ_VBLANK, Vblank);
	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");
	OSTaskCreate(TestTask, (void*)0, &TestTaskStk[99],0);
	//iprintf("\x1b[10;0H OSTaskCreate() init");
	OSStart();



	while(1) {
		iprintf("\x1b[22;0HFrame = %d",frame);
	}

	return 0;
}

void TestTask (void *pdata){
	//pdata = pdata;

	while(1){
		printf("TestTask Init\n");
		OSTimeDly(1);
	}

}
