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



void handle_error(const char *message) {

    // Print the error message to the debug console
    printf("ERROR: %s\n\r", message);

    while (1) {
        // Toggle an LED to indicate an error (assuming LED_PIN is configured)
        GPIOA->ODR ^= LED_PIN;  // Toggle LED
        for (volatile int i = 0; i < 1000000; i++);  // Delay loop
    }

}


const uint8_t Key[] =
{
  0xcf, 0xd4, 0xa4, 0x49, 0x10, 0xc9, 0xe5, 0x67, 0x50, 0x7a, 0xbb, 0x6c, 0xed, 0xe4, 0xfe, 0x60,
  0x1a, 0x7a, 0x27, 0x65, 0xc9, 0x75, 0x5a, 0xa2, 0xcf, 0x6b, 0xa4, 0x81, 0x42, 0x23, 0x81, 0x1a,
  0x26, 0xa8, 0xa1, 0xef, 0x49, 0x9c, 0xeb, 0xd9
};
const uint8_t message[] =
{
  0x3f, 0xb3, 0x01, 0xcb, 0x40, 0x92, 0xf9, 0x62, 0x3a, 0xa5, 0xff, 0xd6, 0x90, 0xd2, 0x2d, 0x65,
  0xd5, 0x6e, 0x5a, 0x1c, 0x33, 0x0b, 0x9c, 0x4a, 0x0d, 0x91, 0x0c, 0x34, 0xe3, 0x91, 0xc9, 0x0a,
  0x76, 0xd5, 0x40, 0x1a, 0x2d, 0x3c, 0xaa, 0x44, 0xb8, 0xc5, 0xd5, 0xae, 0xf3, 0xe9, 0x28, 0xb9,
  0x0d, 0x2e, 0xe2, 0x33, 0xe9, 0xf9, 0xa2, 0xce, 0xc4, 0xa3, 0x2c, 0xd0, 0x19, 0xd0, 0x6a, 0x0d,
  0xc1, 0xfc, 0xb1, 0x12, 0x5f, 0x57, 0x46, 0xa4, 0xfb, 0xd3, 0x21, 0x69, 0xed, 0x7b, 0xf0, 0xe4,
  0xfd, 0x06, 0x5f, 0xa7, 0xc8, 0xac, 0x97, 0xc3, 0x66, 0x38, 0x04, 0x84, 0x49, 0x5f, 0x5c, 0x5b,
  0x68, 0x50, 0xdd, 0x1c, 0x9d, 0x8c, 0xd6, 0x69, 0x4c, 0xf8, 0x68, 0x6e, 0x46, 0x30, 0x8e, 0xd0,
  0xed, 0x1f, 0x5b, 0xdf, 0x98, 0xcd, 0x83, 0x13, 0x39, 0x77, 0x1d, 0xb6, 0x3d, 0xe5, 0xa7, 0xde
};
const uint8_t expected_tag[] =
{
  0x20, 0x15, 0x3b, 0xf8, 0xea, 0x29, 0x53, 0xc4, 0x82, 0x51, 0xeb, 0xcc, 0x41, 0x61, 0xf8, 0xb6,
  0xe2, 0x84, 0x99, 0xe5, 0xc7, 0x6c, 0x24, 0x01, 0x4c, 0xff, 0x4a, 0x9e, 0x2f, 0x62, 0xd2, 0x5c
};



/*Computed data buffer*/
uint8_t computed_tag[CMOX_SHA256_SIZE];

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

	/*1. Initialize ADC*/
	pa1_adc_init();

	/*2. Start conversion*/
	start_conversion();


	/* Initialize cryptolib*/
	if(cmox_initialize(NULL) !=  CMOX_INIT_SUCCESS)
	{
		handle_error("Initialization failed!");
	}



	  /* Compute directly the authentication tag passing all the needed parameters */
	  retval = cmox_mac_compute(CMOX_HMAC_SHA256_ALGO,     /* Use HMAC SHA256 algorithm */
	                            message, sizeof(message),  /* Message to authenticate */
	                            Key, sizeof(Key),          /* HMAC Key to use */
	                            NULL, 0,                   /* Custom data */
	                            computed_tag,              /* Data buffer to receive generated authnetication tag */
	                            sizeof(expected_tag),      /* Expected authentication tag size */
	                            &computed_size);           /* Generated tag size */


	  /* Verify API returned value */
	  if (retval != CMOX_MAC_SUCCESS)
	  {
		  handle_error("Hash Error!");
	  }


	  /* Verify generated data are the expected ones */
	  if (memcmp(expected_tag, computed_tag, computed_size) != 0)
	  {
		  handle_error("Tag Output Size Mismatch!");

	  }

	  retval = cmox_mac_verify(CMOX_HMAC_SHA256_ALGO,     /* Use HMAC SHA256 algorithm */
	                           message, sizeof(message),  /* Message to authenticate */
	                           Key, sizeof(Key),          /* HMAC Key to use */
	                           NULL, 0,                   /* Custom data */
	                           expected_tag,              /* Authentication tag */
	                           sizeof(expected_tag));     /* tag size */

	  if (retval != CMOX_MAC_AUTH_SUCCESS)
	  {
		  handle_error("Auth failed!");

	  }

	while(1)
	{

	}
}

