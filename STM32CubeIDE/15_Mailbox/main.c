#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2;
int32_t semaphore1, semaphore2;

uint32_t sensorValue = 345;
uint32_t sensorReceive;
void Task0(void)
{
	 while(1)
	 {
			osMailBoxSend(sensorValue);
	 }
}

void Task1(void)
{
	 while(1)
	 {
		sensorReceive = osMailBoxReceive();
	 }
}

void Task2(void)
{
	 while(1)
	 {	

	 }
}

int main(void)
{

	HAL_Init();
	ST7735_Init();
	BSP_LED_Init();
	osKernelInit();
	osMailBoxInit();
	osKernelAddThreads(&Task0,&Task1,&Task2);

	
	osKernelLaunch(QUANTA);
	
}
