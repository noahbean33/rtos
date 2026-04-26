#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2;
int32_t semaphore1, semaphore2;

void Task0(void)
{
	 while(1)
	 {
		 osSignalSet(&semaphore2);
		 osSignalCooperativeWait(&semaphore1);
			count0++;
	 }
}

void Task1(void)
{
	 while(1)
	 {
		 osSignalCooperativeWait(&semaphore2);
		 osSignalSet(&semaphore1);
			count1++;
	 }
}

void Task2(void)
{
	 while(1)
	 {	
			count2++;
	 }
}

int main(void)
{
	osSemaphoreInit(&semaphore1,1);
	osSemaphoreInit(&semaphore2,0);
	HAL_Init();
	ST7735_Init();
	BSP_LED_Init();
	osKernelInit();
	osKernelAddThreads(&Task0,&Task1,&Task2);

	
	osKernelLaunch(QUANTA);
	
}
