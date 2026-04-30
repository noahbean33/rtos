#include "uart.h"


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
																		 state_b_function,
																		 state_c_function
																		};
static State_Type current_state;
static int Clock;


int main()
{
	
	USART2_Init();
	while(1)
	{
		state_table[current_state]();
		Clock++;
	}
}

void state_machine_init(void)
{
	current_state = STATE_A;
	Clock =0;
}

void state_a_function(void)
{
	if(Clock == 2)
	{
		current_state = STATE_B;
		printf("This is the execution of STATE A \r\n");
	}
}

void state_b_function(void)
{
		if(Clock == 5)
	{
		current_state = STATE_C;
		printf("This is the execution of STATE B \r\n");
	}
}

void state_c_function(void)
{
		if(Clock == 9)
		{
			Clock = 0;
			current_state = STATE_A;
			printf("This is the execution of STATE C \r\n");
		}	
}