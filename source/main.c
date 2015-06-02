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
int		value[5];


//void printTask();
//void schedTask(void *pdata);
//void TestTask1(void *pdata);
//void TestTask2(void *pdata);
//void TestTask3(void *pdata);
		void Task1(void *pdata);
		void Task2(void *pdata);
		void Task3(void *pdata);
		void Task4(void *pdata);
		void Task5(void *pdata);
		void TaskStart(void *pdata);
		void timerCallBack(void);
		void taskPrint(void);
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
	printf("1\n");
	irqInit();
	printf("2\n");
	OSStatInit();

	printf("3\n");
	timerStart(0, ClockDivider_1024, TIMER_FREQ_64(2), timerCallBack);
	printf("4\n");

	TaskStartCreateTasks();
	printf("5\n");

	while(1){
		printf("6\n");
		OSCtxSwCtr = 0;
		OSTimeDly(1);
		taskPrint();
		printf("7\n");
		//OSTimeDlyHMSM(0,0,1,0);
	}

}

void TaskStartCreateTasks(void){
	INT8U i;

	OSTaskCreate(Task1, (void*)0, &TaskStk[1][TASK_STK_SIZE-1], 1);
	OSTaskCreate(Task2, (void*)0, &TaskStk[2][TASK_STK_SIZE-1], 2);
	OSTaskCreate(Task3, (void*)0, &TaskStk[3][TASK_STK_SIZE-1], 3);
	OSTaskCreate(Task4, (void*)0, &TaskStk[4][TASK_STK_SIZE-1], 4);
	OSTaskCreate(Task5, (void*)0, &TaskStk[5][TASK_STK_SIZE-1], 5);

	/*for(i=0; i<N_TASKS-5; i++){
		//printf("Task Create!\n");
		//TaskData[i] = '0' + i;
		OSTaskCreate(Task, (void*)&TaskData[i], &TaskStk[i][TASK_STK_SIZE-1], i+1);
	}*/
}

void Task1(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[0] = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		//iprintf("\x1b[2;0H task1 x : %d\n",x);
		OSTimeDly(1);
	}
}

void Task2(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[1] = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		//iprintf("\x1b[3;0H task2 x : %d\n",x);
		OSTimeDly(1);
	}
}

void Task3(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[2] = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		//iprintf("\x1b[4;0H task3 x : %d\n",x);
		OSTimeDly(1);
	}
}

void Task4(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[3] = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		//iprintf("\x1b[5;0H task4 x : %d\n",x);
		OSTimeDly(1);
	}
}

void Task5(void *pdata){
	INT8U err;

	printf("task create\n");

	while(1){
		OSSemPend(RandomSem, 0, &err);
		value[4] = rand()%10+1;
		OSSemPost(RandomSem);
		//화면출력
		//iprintf("\x1b[6;0H task5 x : %d\n",x);
		OSTimeDly(1);
	}
}

void taskPrint(void){
	INT8U index;

//	for(index=0; index<5; index++){
//		printf("value[%d] = %d\n",index,value[index]);
//	}
	printf("value[0] = %d\n"
			"value[1] = %d\n"
			"value[2] = %d\n"
			"value[3] = %d\n"
			"value[4] = %d\n"
			"OSCPUUsage = %d\n",value[0],value[1],value[2],value[3],value[4],OSCPUUsage);
	//printf("OSCPUUsage = %d\n",OSCPUUsage);
}



void timerCallBack() {
	puts("call\n");
//	OSStatInit();
	OSTimeTick();
	OS_Sched();
}
