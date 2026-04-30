#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2;
int32_t semaphore1, semaphore2;

void Task0(void)
{
	 while(1)
	 {
			count0++;
			BSP_LED_greenToggle();
			osSignalWait(&semaphore2);
		 ST7735_DrawString(3,3, "THIS IS THREAD 0", GREEN);
		 osSignalSet(&semaphore1);
	 }
}

void Task1(void)
{
	 while(1)
	 {
			
			count1++;
			BSP_LED_blueToggle();
		  osSignalWait(&semaphore1);
		  ST7735_DrawString(3,7, "THIS IS THREAD 1", BLUE_);
		 osSignalSet(&semaphore2);
	 }
}

void Task2(void)
{
	 while(1)
	 {
			
			count2++;
			BSP_LED_orangeToggle();
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
