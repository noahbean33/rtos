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


	/*TEST 1*/
	encrypt_ceasar_cipher(plaintext, shift,encryptedtext,length);
	decrypt_ceasar_cipher(encryptedtext, shift,decryptedtext,length);


	/*TEST 2*/
//	sensor_value = 2789;
//
//	encrypted_u16 = encrypt_ceasar_cipher_u16(sensor_value,shift_u16);
//	decrypted_u16 = decrypt_ceasar_cipher_u16(encrypted_u16,shift_u16);


	/*TEST 4*/
	for( int i = 0; i < 5; i++)
	{
		encrypted_values[i] = encrypt_ceasar_cipher_u16(sensor_values[i],shift_u16);

	}

	for( int i = 0; i < 5; i++)
	{
		decrypted_values[i] = decrypt_ceasar_cipher_u16(encrypted_values[i],shift_u16);

	}
	while(1)
	{


		/*TEST 3*/
//		sensor_value = adc_read();
//
//		encrypted_u16 = encrypt_ceasar_cipher_u16(sensor_value,shift_u16);
//		decrypted_u16 = decrypt_ceasar_cipher_u16(encrypted_u16,shift_u16);
	}
}

