#include "uart.h"
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common

#define MAX_STATE		6

enum states
{	
	STATE_A	=0,
	STATE_B,
	STATE_C
};

typedef enum states State_Type;

void state_a_function(void);
void state_b_function(void);
void state_c_function(void);
void state_machine_init(void);

static void(*state_table[])(void) = {state_a_function,
																		 state_a_function,
																		 state_b_function,
																		 state_a_function,
																		 state_c_function,
																		 state_b_function
																		};
uint8_t current_state = 0;
static int Clock;


int main()
{
	HAL_Init();
	
	USART2_Init();
	while(1)
	{
		state_table[current_state]();
		HAL_Delay(1000);
		Clock++;
		if(current_state == MAX_STATE)current_state =0;
	}
}

void state_machine_init(void)
{
//	current_state = STATE_A;
	Clock =0;
}

uint32_t stateA_prev_time = 0;
uint32_t stateA_now;
float stateA_delta;

void state_a_function(void)
{
	if(Clock % 2)
	{
		current_state++;
		
		stateA_now = HAL_GetTick();
		stateA_delta = stateA_now - stateA_prev_time;
		/*To seconds*/
		stateA_delta /= 1000;
		stateA_prev_time = stateA_now;
		printf("This is the output of STATE A: %f seconds ago\n\r",stateA_delta);
		//printf("This is the execution of STATE A \r\n");
	}
}
uint32_t stateB_prev_time = 0;
uint32_t stateB_now;
float stateB_delta;

void state_b_function(void)
{
		if(Clock % 5)
	{
		current_state++;
		
		stateB_now = HAL_GetTick();
		stateB_delta = stateB_now - stateB_prev_time;
		/*To seconds*/
		stateB_delta /= 1000;
		stateB_prev_time = stateB_now;
		printf("This is the output of STATE B: %f seconds ago\n\r",stateB_delta);
		//printf("This is the execution of STATE B \r\n");
	}
}

uint32_t stateC_prev_time = 0;
uint32_t stateC_now;
float stateC_delta;

void state_c_function(void)
{
		if(Clock % 9)
		{
			current_state++;
			Clock =0;
			stateC_now = HAL_GetTick();
			stateC_delta = stateC_now - stateC_prev_time;
			/*To seconds*/
			stateC_delta /= 1000;
			stateC_prev_time = stateC_now;
			printf("This is the output of STATE C: %f seconds ago\n\r",stateC_delta);
			
			//printf("This is the execution of STATE C \r\n");
		}	
}


void SysTick_Handler(void)
{
	HAL_IncTick();					
}