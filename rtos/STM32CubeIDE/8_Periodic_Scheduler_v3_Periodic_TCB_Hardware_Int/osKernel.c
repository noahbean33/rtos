/*
   -schedular
	 -Thread Control Block
	 -Stack
*/

#include "osKernel.h"


#define NUM_OF_THREADS 3
#define STACKSIZE 100
#define NUM_PERIODIC_TASK		5

typedef void(*taskT)(void);
#define NULL (void*)0

#define BUS_FREQ		16000000

uint32_t MILLIS_PRESCALER;
void osSchedularLaunch(void);

/*Systick priority register*/
#define SYSPRI3         (*((volatile uint32_t *)0xE000ED20))
#define INTCTRL         (*((volatile uint32_t *)0xE000ED04))
//#define NVIC_INT_CTRL_R	(*((volatile uint32_t *)0xE000ED04))

//#define PERIOD					100

void osPeriodicTask_Init(void(*task)(void), uint32_t freq, uint8_t priority);
typedef struct
{
	taskT task;
	uint32_t period;
	uint32_t timeLeft;
	
}periodicTaskT;

static periodicTaskT periodicTask[NUM_PERIODIC_TASK];
static uint32_t TimeMsec;
static uint32_t MaxPeriod;
static int32_t NumPeriodicThreads =0;

struct tcb{
	
  int32_t *stackPt;
	struct tcb *nextPt;
	
};

typedef struct tcb tcbType;
tcbType tcbs[NUM_OF_THREADS];
tcbType *currentPt;

int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

void osKernelStackInit(int i){
   tcbs[i].stackPt =  &TCB_STACK[i][STACKSIZE -16];//StackPointer
	 TCB_STACK[i][STACKSIZE -1] =  0x01000000;
	
	TCB_STACK[i][STACKSIZE-3] = 0x14141414;  //R14
	TCB_STACK[i][STACKSIZE-4] = 0x12121212;  //R12
	TCB_STACK[i][STACKSIZE-5] = 0x03030303;  //R3
	TCB_STACK[i][STACKSIZE-6] = 0x02020202;  //R2
	TCB_STACK[i][STACKSIZE-7] = 0x01010101;  //R1
	TCB_STACK[i][STACKSIZE-8] = 0x00000000;  //R0
	  /*We have to save manually*/	
	TCB_STACK[i][STACKSIZE-9] = 0x11111111;  //R11
	TCB_STACK[i][STACKSIZE-10] = 0x10101010; //R10
	TCB_STACK[i][STACKSIZE-11] = 0x09090909; //R9
	TCB_STACK[i][STACKSIZE-12] = 0x08080808; //R8
	TCB_STACK[i][STACKSIZE-13] = 0x07070707; //R7
	TCB_STACK[i][STACKSIZE-14] = 0x06060606; //R6
	TCB_STACK[i][STACKSIZE-15] = 0x05050505; //R5
	TCB_STACK[i][STACKSIZE-16] = 0x04040404; //R4
}

uint8_t osKernelAddPeriod_Thread(void(*task)(void),uint32_t period)
{
		if(NumPeriodicThreads == NUM_OF_THREADS || period ==0)
		{
			return 0;
		}
		periodicTask[NumPeriodicThreads].task = task;
		periodicTask[NumPeriodicThreads].period = period;
		periodicTask[NumPeriodicThreads].timeLeft = period -1;
		
		NumPeriodicThreads++;
		
		return 1;
		
}

void periodic_event_execute(void)
{
	int i;
	for(i = 0; i < NumPeriodicThreads; i++)
	{
		if(periodicTask[i].timeLeft ==0)
		{
			periodicTask[i].task();
			periodicTask[i].timeLeft = periodicTask[i].period -1 ;
			
		}
		else
		{
			periodicTask[i].timeLeft--;
		}
	}
}

uint8_t osKernelAddThreads(void(*task0)(void), void(*task1)(void),void(*task2)(void))
{
	__disable_irq();
	
	tcbs[0].nextPt = &tcbs[1];
	tcbs[1].nextPt = &tcbs[2];
	tcbs[2].nextPt = &tcbs[0];
	
	
	osKernelStackInit(0);
	TCB_STACK[0][STACKSIZE-2] = (int32_t)(task0);
	
  osKernelStackInit(1);
	TCB_STACK[1][STACKSIZE-2] = (int32_t)(task1);
	
  osKernelStackInit(2);
	TCB_STACK[2][STACKSIZE-2] = (int32_t)(task2);
	
	currentPt =&tcbs[0];
	
	__enable_irq();
	
		return 1;

	
}


void osKernelInit(void)
{
	MILLIS_PRESCALER =  (BUS_FREQ/1000);
	/*Frequency options: 1Hz, 10Hz, 100Hz, 1kHz, 10kHz, 100kHz*/
	osPeriodicTask_Init(periodic_event_execute, 1000,6);
}


void osKernelLaunch(uint32_t quanta)
{
	 SysTick->CTRL =0;
	 SysTick->VAL  =0;
	 SysTick->LOAD =  (quanta*MILLIS_PRESCALER)-1;
	
	 SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
	 SysTick->CTRL =  0x00000007;
	
	osSchedularLaunch();
	
}

void osThreadYield(void)
{
	SysTick->VAL = 0;
	INTCTRL = 0x04000000; 	//trigger SysTick
}

void SysTick_Handler(void)
{
	// Trigger PendSV
	INTCTRL = 0x10000000;
}

uint32_t period_tick;

void osSchedulerPeriodicRR(void)
{

	currentPt = currentPt->nextPt;
}

void(*PeriodicTask)(void);

void osPeriodicTask_Init(void(*task)(void), uint32_t freq, uint8_t priority)
{
	__disable_irq();
	PeriodicTask = task;
	RCC->APB1ENR |= 1;
	TIM2->PSC = 16-1;		/*16 000 000 / 16 = 1000 000*/
	TIM2->ARR = (1000000/freq) -1;
	TIM2->CR1 = 1;
	
	TIM2->DIER |= 1;
	NVIC_SetPriority(TIM2_IRQn,priority);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{
	TIM2->SR = 0;
	(PeriodicTask)();
}