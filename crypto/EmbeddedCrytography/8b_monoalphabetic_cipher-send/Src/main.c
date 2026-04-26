#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "monoalphabetic_cipher.h"


#define ENCRYPT_TEXT_LEN 6   // +1 for the null-terminator

// A 26-character key representing the shuffled alphabet
char key[] = "DFGHIJKLMNOPQRSTUVWXYZABCETSMN";

char encryptedtext[ENCRYPT_TEXT_LEN];
char decryptedtext[ENCRYPT_TEXT_LEN];

char plaintext[] = "HELLO";


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



	encrypt_monoalphabetic_cipher(plaintext, key, encryptedtext);


	while(1)
	{

		 for(int i = 0;  i < (ENCRYPT_TEXT_LEN -1); i++)
		 {
			 uart_write(encryptedtext[i]);
		 }
		 uart_write_crlf();

		 delay(1);


	}
}

