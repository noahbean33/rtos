#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "cmox_crypto.h"
#include <string.h>




#define  GPIOAEN		(1U<<0)
#define  PIN5			(1U<<5)
#define  LED_PIN		PIN5

bool btn_state;
uint32_t sensor_value;


/** Extract from NIST Special Publication 800-38A
  * F.2.1 CBC-AES128.Encrypt
    Key 2b7e151628aed2a6abf7158809cf4f3c
    IV 000102030405060708090a0b0c0d0e0f

    Block #1
    Plaintext 6bc1bee22e409f96e93d7e117393172a
    Input Block 6bc0bce12a459991e134741a7f9e1925
    Output Block 7649abac8119b246cee98e9b12e9197d
    Ciphertext 7649abac8119b246cee98e9b12e9197d

    Block #2
    Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
    Input Block d86421fb9f1a1eda505ee1375746972c
    Output Block 5086cb9b507219ee95db113a917678b2
    Ciphertext 5086cb9b507219ee95db113a917678b2

    Block #3
    Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
    Input Block 604ed7ddf32efdff7020d0238b7c2a5d
    Output Block 73bed6b8e3c1743b7116e69e22229516
    Ciphertext 73bed6b8e3c1743b7116e69e22229516

    Block #4
    Plaintext f69f2445df4f9b17ad2b417be66c3710
    Input Block 8521f2fd3c8eef2cdc3da7e5c44ea206
    Output Block 3ff1caa1681fac09120eca307586e1a7
    Ciphertext 3ff1caa1681fac09120eca307586e1a7
  */


const uint8_t Key[] =
{
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
const uint8_t IV[] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};
const uint8_t plain_text[] =
{
  0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
  0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
  0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
  0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
const uint8_t expected_cipher_text[] =
{
  0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
  0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
  0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
  0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7
};


uint8_t computed_cipher_text[sizeof(expected_cipher_text)];
uint8_t computed_plain_text[sizeof(plain_text)];

int main()
{

	cmox_cipher_retval_t  retval;
	size_t computed_size;

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

	/*1. Initialize cryptolib*/
	if(cmox_initialize(NULL) !=  CMOX_INIT_SUCCESS)
	{
		while(1)
		{
		printf("ERROR: Init failed\r\n");
		delay(500);

		}
	}

	/*2. Encrypt the plaintext*/

	retval = cmox_cipher_encrypt(CMOX_AES_CBC_ENC_ALGO,
								plain_text, sizeof(plain_text),
								Key,sizeof(Key),
								IV,sizeof(IV),
								computed_cipher_text, &computed_size);

	/*3.Verify return value*/
	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		while(1)
		{
		printf("ERROR: Encryption failed\r\n");
		delay(500);

		}
	}

	/*4. Compare ciphertext to expected ciphertext*/

	if(computed_size!= sizeof(expected_cipher_text))
	{
		printf("ERROR: Size mismatch!\n\r");
	}

	if(memcmp(expected_cipher_text,computed_cipher_text,computed_size) != 0)
	{
		while(1)
		{
		printf("ERROR: Wrong  Encryption output\r\n");
		delay(500);

		}
	}
	/*5. Decrypt the ciphertext*/

	retval = cmox_cipher_decrypt(CMOX_AES_CBC_DEC_ALGO,
			                    computed_cipher_text, sizeof(computed_cipher_text),
								Key,sizeof(Key),
								IV,sizeof(IV),
								computed_plain_text, &computed_size);

	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		while(1)
		{
		printf("ERROR: decryption failed\r\n");
		delay(500);

		}
	}

	/*4. Compare plaintext to decrypted text*/
	if(computed_size!= sizeof(plain_text))
	{
		printf("ERROR: Size mismatch!\n\r");
	}

	if(memcmp(plain_text,computed_plain_text,computed_size) != 0)
	{
		while(1)
		{
		printf("ERROR: Wrong  Decryption output\r\n");
		delay(500);

		}
	}
	while(1)
	{

	}
}

