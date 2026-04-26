#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2;

void Task0(void)
{
	 while(1){
	 count0++;
	 osThreadYield();
	 BSP_LED_redToggle();
	 }
}

void Task1(void)
{
	 while(1){
	 count1++;
	 BSP_LED_blueToggle();
	 }
}

void Task2(void)
{
	 while(1){
	  count2++;
		BSP_LED_orangeToggle();
	 }
}
int main(void)
{
	BSP_LED_Init();
	osKernelInit();
	osKernelAddThreads(&Task0,&Task1,&Task2);
	osKernelLaunch(QUANTA);
	
}