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

#define AES_BLOCK_SIZE   16                  // AES block size in bytes

#define ADC_SAMPLE_COUNT  1                // Number of ADC samples
#define BYTES_PER_SAMPLE  sizeof(uint32_t)  // 4 bytes per 32-bit ADC value


uint32_t adc_values[ADC_SAMPLE_COUNT];       // Stores original ADC readings
uint32_t decrypted_adc_values[ADC_SAMPLE_COUNT];

uint8_t plain_text_bytes[ADC_SAMPLE_COUNT * BYTES_PER_SAMPLE];  // 8-bit representation for encryption
uint8_t cipher_text_bytes[(ADC_SAMPLE_COUNT * BYTES_PER_SAMPLE) + AES_BLOCK_SIZE]; // Buffer for ciphertext with padding
uint8_t decrypted_bytes[(ADC_SAMPLE_COUNT * BYTES_PER_SAMPLE) + AES_BLOCK_SIZE]; // Buffer for decrypted data




void decompose_adc_values(void) {

    for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
        uint32_t adc_value = adc_values[i];
        plain_text_bytes[i * 4]     = (adc_value >> 24) & 0xFF;  // MSB
        plain_text_bytes[i * 4 + 1] = (adc_value >> 16) & 0xFF;
        plain_text_bytes[i * 4 + 2] = (adc_value >> 8) & 0xFF;
        plain_text_bytes[i * 4 + 3] = adc_value & 0xFF;          // LSB
    }
}

void recombine_adc_values(void) {
    for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
    	decrypted_adc_values[i] = (decrypted_bytes[i * 4] << 24) |
                       (decrypted_bytes[i * 4 + 1] << 16) |
                       (decrypted_bytes[i * 4 + 2] << 8) |
                       decrypted_bytes[i * 4 + 3];
    }
}

void handle_error(const char *message) {

    // Print the error message to the debug console
    printf("ERROR: %s\n\r", message);

    while (1) {
        // Toggle an LED to indicate an error (assuming LED_PIN is configured)
        GPIOA->ODR ^= LED_PIN;  // Toggle LED
        for (volatile int i = 0; i < 1000000; i++);  // Delay loop
    }

}



// Function to apply PKCS#7 padding
size_t apply_padding(const uint8_t *input, size_t input_len, uint8_t *output) {

	/*Calculate the padded length*/
    size_t padded_len = ((input_len/AES_BLOCK_SIZE) + 1 ) * AES_BLOCK_SIZE;


    /*Copy the input into the output buffer*/
    memcpy(output, input, input_len);

    uint8_t padding_value = padded_len - input_len;

    /*Add padding*/
    for (size_t i = input_len; i < padded_len; i++) {
        output[i] = padding_value;
    }

    return padded_len;
}


// Function to remove PKCS#7 padding
size_t remove_padding(const uint8_t *input, size_t input_len, uint8_t *output) {

	if( input_len == 0 || input_len % AES_BLOCK_SIZE != 0)
	{
        handle_error("Invalid input length for  padding");

	}

	/*Retreive the padding value from the last byte*/
    uint8_t padding_value = input[input_len - 1];

    /*Valid the padding value */
    if (padding_value > AES_BLOCK_SIZE || padding_value == 0) {
        handle_error("Invalid padding");
    }

    /*Verify that all padding bytes are correct*/
    for (size_t i = input_len - padding_value ; i < input_len; i++) {

    	if(input[i] != padding_value ){
            handle_error("Invalid padding content ");
    	}
    }

    /*Calculate the orignal length to remove padding*/
    size_t original_len = input_len - padding_value;

    /*Copy the original data to the output buffer*/
    memcpy(output, input, original_len);


    return original_len;
}

const uint8_t Key[] =
{
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
const uint8_t IV[] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};




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

	/*3. Read ADC value*/
	for(int i =0 ;i < ADC_SAMPLE_COUNT; i++)
	{
		adc_values[i] = adc_read();
		delay(300);
	}
	/*4. Decompose adc values into bytes*/
	decompose_adc_values();

	 /*Apply padding*/
     uint8_t padded_plain_text[(ADC_SAMPLE_COUNT * BYTES_PER_SAMPLE) + AES_BLOCK_SIZE];
     size_t padded_len = apply_padding(plain_text_bytes,sizeof(plain_text_bytes),padded_plain_text);

	/*5. Initialize cryptolib*/
	if(cmox_initialize(NULL) !=  CMOX_INIT_SUCCESS)
	{
		handle_error("Initialization failed!");
	}

	/*6. Encrypt the plaintext*/
	retval = cmox_cipher_encrypt(CMOX_AES_CBC_ENC_ALGO,
			                    padded_plain_text, padded_len,
								Key,sizeof(Key),
								IV,sizeof(IV),
								cipher_text_bytes, &computed_size);

	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		handle_error("Encryption failed!");

	}


	/*7. Decrypt the bytes*/
	retval = cmox_cipher_decrypt(CMOX_AES_CBC_DEC_ALGO,
			                     cipher_text_bytes, computed_size,
								Key,sizeof(Key),
								IV,sizeof(IV),
								decrypted_bytes, &computed_size);
	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		handle_error("Decryption failed!");

	}

//	/*Remove padding from encryped buytes*/
	uint8_t unpadded_plain_text[sizeof(plain_text_bytes)];
	size_t original_len = remove_padding(decrypted_bytes,computed_size,unpadded_plain_text);



	/*Verify the decrypted data matches the origianal plaintext*/
	if(memcmp(plain_text_bytes,unpadded_plain_text,original_len) !=0)
	{
		handle_error("Decrypted value  mismatch!");

	}

	/*8.  Recombine decrypted bytes into 32-bit ADC values*/
	recombine_adc_values();


	printf("*************************************************************\r\n");
	for(int i =0 ;i < ADC_SAMPLE_COUNT; i++)
	{
	   printf("Decrypted sensor value[%d]: %d\r\r",i, (int)decrypted_adc_values[i]);
	   printf("Original sensor value[%d]: %d\r\r",i, (int)adc_values[i]);

	}
	printf("*************************************************************\r\n");



	while(1)
	{

	}
}

