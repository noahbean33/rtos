#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2,sporadic_counter;
int32_t edgeSem;

void SporadicTask(void)
{
	 while(1)
	 {
			osSignalWait(&edgeSem);
		  sporadic_counter++;
	 }
}
/**/
void Task1(void)
{
	 while(1)
	 {
			BSP_LED_blueToggle();
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

	BSP_LED_Init();
	osKernelInit();
	osEdgeTriggerInit(&edgeSem);
	osKernelAddThreads(&SporadicTask,&Task1,&Task2);

	
	osKernelLaunch(QUANTA);
	
}

void EXTI0_IRQHandler(void)
{
	osSignalSet(&edgeSem);
	EXTI->PR = 0x001;
}
