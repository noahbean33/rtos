#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "ceasar_cipher.h"


#define ENCRYPT_TEXT_LEN 6   // +1 for the null-terminator


uint16_t shift_u16 =  4298;

uint8_t encryptedtext[ENCRYPT_TEXT_LEN];
uint8_t decryptedtext[ENCRYPT_TEXT_LEN];


uint16_t encrypted_u16;
uint16_t decrypted_u16;

uint8_t plaintext[] ="HELLO";
uint8_t shift =  3;
uint8_t length =  5;


uint16_t sensor_value;

/*Data for TEST 4*/
uint16_t sensor_values[5] = {2408,3001,699,877,2098};
uint16_t encrypted_values[5];
uint16_t decrypted_values[5];


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


   char encryptedtext[] = "Wklv lv d whvw phvvdjh";

   for(int key = 1; key <= 25; key++)
   {
	   printf("Key %d: \n",key);
	   decrypt_ceasar_hack_test(encryptedtext,key);
   }

	while(1)
	{



	}
}

