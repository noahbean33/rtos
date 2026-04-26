#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "ceasar_cipher.h"


#define ENCRYPT_TEXT_LEN 6   // +1 for the null-terminator



uint8_t encryptedtext[ENCRYPT_TEXT_LEN];
uint8_t decryptedtext[ENCRYPT_TEXT_LEN];



uint8_t plaintext[] ="HELLO";
uint8_t shift =  3;
uint8_t length =  5;


int main()
{
	/*Enable FPU*/
	fpu_enable();

	/*Initialize debug UART*/
	debug_uart_init();

	/*Initialize timebase*/
	timebase_init();

	/*Initialize LED*/
	led_init();

	/*Initialize Push button*/
	button_init();

	/*Initialize ADC*/
	pa1_adc_init();

	/*Start conversion*/
	start_conversion();


	encrypt_ceasar_cipher(plaintext, shift,encryptedtext,length);





	while(1)
	{
       for(int i = 0;  i < length;  i++)
       {
    	   uart_write(encryptedtext[i]);
       }

       uart_write_crlf();

       /*Delay 1 second*/
        delay(1);
	}
}

