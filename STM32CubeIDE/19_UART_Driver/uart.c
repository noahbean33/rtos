#include "uart.h"


void USART2_Init(void)
{

//1 Enable clock access to portA
	RCC->AHB1ENR |= 0x01;
//2 Enable clock access to uart
	RCC->APB1ENR |= 0x20000;
//3 Enable pins for alternate functions PA2(USART2_TX), PA3(USART2_RX)
	GPIOA->MODER &= ~0x00F0;
	GPIOA->MODER |=  0x00A0;
//4 Configure type of alternate function
	GPIOA->AFR[0] &= ~0xFF00;
	GPIOA->AFR[0] |=  0x7700;


// Configure UART
	USART2->BRR	= 0x0683;
	USART2->CR1	= 0x000C;
	USART2->CR2 = 0x0000;
	USART2->CR3 = 0x0000;
	USART2->CR1 |=  0x2000;
}

// This configuration by UART needs a tool with FT232BL USB to TTL
int USART2_Write(int ch)
{
	while(!(USART2->SR & 0x0080)){}
	USART2->DR = (ch & 0xFF);
	return ch;	
}

int USART2_Read(void)
{
	while(!(USART2->SR & 0x0020)){}
	return USART2->DR;
}



//	struct __FILE{int handle;};
//__asm(".global __use_no_semihosting");
FILE __stdout;
FILE __stdin;
FILE __stderr;
	
	int fgetc(FILE *f)
	{
		int c;
		c = USART2_Read();
		if(c=='\r')
		{
			USART2_Write(c);
			c = '\n';
		}
		USART2_Write(c);
		return c;
	}
	
	int fputc(int c, FILE *stream)
	{
		return USART2_Write(c);
	}
	
	int ferror (FILE *stream)
	{
		return 1;
	}
	
	long int ftell(FILE *stream)
	{
		return 1;
	}
	int fclose(FILE *f)
	{
		return 1;
	}	
	int fseek(FILE *f, long nPos, int nMode)
	{
		return 0;
	}
	int fflush(FILE *f)
	{
		return 1;
	}
	


int n;
static char str[80];
void test_setup(void)
{
	printf("Please enter a number");
	scanf("%d",&n);
	printf("the number entered is: %d\r\n",n);
	printf("Please type a character string: ");
	gets(str);
	printf("The character string entered is : ");
	puts(str);
	printf("\r\n");

}