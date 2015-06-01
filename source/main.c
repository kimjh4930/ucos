#include <nds.h>
#include <stdio.h>
#include "includes.h"

#ifndef UCOS_II
#define UCOS_II
#include "ucos_ii.h"
#endif

#define TASK_STK_SIZE		100
#define N_TASKS				 10

//OS_STK TestTaskStksched[100];
//OS_STK TestTaskStk1[100];
//OS_STK TestTaskStk2[100];
//OS_STK TestTaskStk3[100];

OS_STK	TaskStk[N_TASKS][TASK_STK_SIZE];
OS_STK	TaskStartStk[TASK_STK_SIZE];
char	TaskData[N_TASKS];
OS_EVENT *RandomSem;


//void printTask();
//void schedTask(void *pdata);
//void TestTask1(void *pdata);
//void TestTask2(void *pdata);
//void TestTask3(void *pdata);
		void Task(void *pdata);
		void TaskStart(void *pdata);
		void timerCallBack(void);
static 	void TaskStartCreateTasks(void);

int timerCall = 0;

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	consoleDemoInit();

	OSInit();
	iprintf("\x1b[1;0H ----OSInit() init----\n");

	//irqInit();		//libnds interrupt system 초기화

	RandomSem = OSSemCreate(1);	//Random 숫자 세마포어

	OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE-1], 0);
	//OSTaskCreate(schedTask, (void*) 0, &TestTaskStksched[99], 1);
	//OSTaskCreate()함수를 거치면 TASK READY 상태가 되어 실행할 준비를 마친다.
	OSStart();

	return 0;
}

void TaskStart(void *pdata){
	/*
	 * 디스플레이 초기화
	 * OSTickISR 설치
	 * OSStatInit(); //통계 Task 설치
	 * TaskStartCreateTasks();	//모든 태스크 생성
	 */

	/* 통계 Task 초기화
	 * IDLE 태스크 외에 아무 태스크도 실행되지 않은 상태에서 IDLE 카운터 값을 얼마나 증가할 수 있는지를 알아냄.
	 */
	timerStart(0, ClockDivider_1024, TIMER_FREQ_64(2), timerCallBack);
	//OSStatInit();
	TaskStartCreateTasks();

	while(1){
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		//OSTimeDlyHMSM(0,0,1,0);
	}

}

void TaskStartCreateTasks(void){
	INT8U i;

	for(i=0; i<N_TASKS; i++){
		//printf("Task Create!\n");
		//TaskData[i] = '0' + i;
		OSTaskCreate(Task, (void*)0/*&TaskData[i]*/, &TaskStk[i][TASK_STK_SIZE-1], 1);
	}
}

void Task(void *pdata){
	int x;
	INT8U err;

	while(1){
		OSSemPend(RandomSem, 0, &err);
		x = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		printf("x : %d\n",x);
		OSTimeDly(1);
	}
}



void timerCallBack() {
	puts("call\n");
	OSTimeTick();
	OS_Sched();
}
