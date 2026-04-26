#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "vigenere_cipher.h"


#define ENCRYPT_TEXT_LEN 15   // +1 for the null-terminator



char encryptedtext[ENCRYPT_TEXT_LEN];
char decryptedtext[ENCRYPT_TEXT_LEN];

char plaintext[] = "Hello World";
char keyword[] = "kEy";

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


	encrypt_vigenere_cipher2(plaintext,keyword,encryptedtext);
	decrypt_vigenere_cipher2(encryptedtext,keyword,decryptedtext);

	while(1)
	{



	}
}

