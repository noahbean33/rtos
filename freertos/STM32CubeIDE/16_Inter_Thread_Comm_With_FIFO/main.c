#include "osKernel.h"
#include "STM32F4_RTOS_BSP.h"

#define	QUANTA	10


uint32_t count0,count1,count2;
int32_t semaphore1, semaphore2;

uint32_t sensorValue;
void Task0(void)
{
	 while(1)
	 {
			osFifoPut(BSP_Sensor_Read());
	 }
}

void Task1(void)
{
	 while(1)
	 {
			sensorValue = osFifoGet();
		 if(sensorValue > 2000)
		 {
			BSP_LED_blueOn();
			BSP_LED_orangeOn();
			BSP_LED_greenOn();
			BSP_LED_redOn();
		 }
		 else
		 {
			BSP_LED_blueOff();
			BSP_LED_orangeOff();
			BSP_LED_greenOff();
			BSP_LED_redOff();
		 }
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
	BSP_ADC1_Init();
	osFifoInit();
	osKernelInit();
	osMailBoxInit();
	osKernelAddThreads(&Task0,&Task1,&Task2);

	
	osKernelLaunch(QUANTA);
	
}
