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

#define AES_256_KEY_SIZE		32  // 32bytes =   256bits
#define AES_256_IV_SIZE		    16  // 16bytes


#define AES_BLOCK_SIZE   16                  // AES block size in bytes

#define ADC_SAMPLE_COUNT  8                // Number of ADC samples
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

uint8_t Key[AES_256_KEY_SIZE] ={};
uint8_t IV[AES_256_IV_SIZE] = {};


const uint8_t entropy[] =
{
  0x4c, 0xfb, 0x21, 0x86, 0x73, 0x34, 0x6d, 0x9d, 0x50, 0xc9, 0x22, 0xe4, 0x9b, 0x0d, 0xfc, 0xd0,
  0x90, 0xad, 0xf0, 0x4f, 0x5c, 0x3b, 0xa4, 0x73, 0x27, 0xdf, 0xcd, 0x6f, 0xa6, 0x3a, 0x78, 0x5c
};

const uint8_t nonce[] =
{
  0x01, 0x69, 0x62, 0xa7, 0xfd, 0x27, 0x87, 0xa2, 0x4b, 0xf6, 0xbe, 0x47, 0xef, 0x37, 0x83, 0xf1
};
const uint8_t personalization[] =
{
  0x88, 0xee, 0xb8, 0xe0, 0xe8, 0x3b, 0xf3, 0x29, 0x4b, 0xda, 0xcd, 0x60, 0x99, 0xeb, 0xe4, 0xbf,
  0x55, 0xec, 0xd9, 0x11, 0x3f, 0x71, 0xe5, 0xeb, 0xcb, 0x45, 0x75, 0xf3, 0xd6, 0xa6, 0x8a, 0x6b
};

const uint8_t entropy_input_reseed[] =
{
  0xb7, 0xec, 0x46, 0x07, 0x23, 0x63, 0x83, 0x4a, 0x1b, 0x01, 0x33, 0xf2, 0xc2, 0x38, 0x91, 0xdb,
  0x4f, 0x11, 0xa6, 0x86, 0x51, 0xf2, 0x3e, 0x3a, 0x8b, 0x1f, 0xdc, 0x03, 0xb1, 0x92, 0xc7, 0xe7
};


const uint8_t known_random[] =
{
  0xa5, 0x51, 0x80, 0xa1, 0x90, 0xbe, 0xf3, 0xad, 0xaf, 0x28, 0xf6, 0xb7, 0x95, 0xe9, 0xf1, 0xf3,
  0xd6, 0xdf, 0xa1, 0xb2, 0x7d, 0xd0, 0x46, 0x7b, 0x0c, 0x75, 0xf5, 0xfa, 0x93, 0x1e, 0x97, 0x14,
  0x75, 0xb2, 0x7c, 0xae, 0x03, 0xa2, 0x96, 0x54, 0xe2, 0xf4, 0x09, 0x66, 0xea, 0x33, 0x64, 0x30,
  0x40, 0xd1, 0x40, 0x0f, 0xe6, 0x77, 0x87, 0x3a, 0xf8, 0x09, 0x7c, 0x1f, 0xe9, 0xf0, 0x02, 0x98
};



/* Computed data buffer */
uint8_t computed_random[sizeof(known_random)];

/*DRBG context handle*/
cmox_ctr_drbg_handle_t con_drbg_ctx;

void generate_key_and_iv(void) {

    cmox_drbg_retval_t retval;

    /* General DRBG context */
    cmox_drbg_handle_t *drbg_ctx;

    /* Construct a DRBG context for CTR-DRBG with AES-256 */
    drbg_ctx = cmox_ctr_drbg_construct(&con_drbg_ctx, CMOX_CTR_DRBG_AES256);
    if (drbg_ctx == NULL) {
        handle_error("DRBG context construction failed");
    }

    /* Initialize DRBG context with entropy and nonce */
    retval = cmox_drbg_init(drbg_ctx, entropy, sizeof(entropy),  personalization, sizeof(personalization), nonce, sizeof(nonce));
    if (retval != CMOX_DRBG_SUCCESS) {
        handle_error("DRBG initialization failed");
    }


    /* Generate the AES Key */
    retval = cmox_drbg_generate(drbg_ctx, NULL, 0, Key, AES_256_KEY_SIZE);
    if (retval != CMOX_DRBG_SUCCESS) {
        handle_error("Key generation failed");
    }

    /* Generate the AES IV */
    retval = cmox_drbg_generate(drbg_ctx, NULL, 0, IV, AES_256_IV_SIZE);
    if (retval != CMOX_DRBG_SUCCESS) {
        handle_error("IV generation failed");
    }

    /* Clean up DRBG context */
    retval = cmox_drbg_cleanup(drbg_ctx);
    if (retval != CMOX_DRBG_SUCCESS) {
        handle_error("DRBG cleanup failed");
    }

}
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


	//------------------------------------------------------------------------

	/*. Read ADC value*/
	for(int i =0 ;i < ADC_SAMPLE_COUNT; i++)
	{
		adc_values[i] = adc_read();
		delay(300);
	}

	/*. Decompose adc values into bytes*/
	decompose_adc_values();

	 /*Apply padding*/
     uint8_t padded_plain_text[(ADC_SAMPLE_COUNT * BYTES_PER_SAMPLE) + AES_BLOCK_SIZE];
     size_t padded_len = apply_padding(plain_text_bytes,sizeof(plain_text_bytes),padded_plain_text);


     /*Generate keys and iv*/
    generate_key_and_iv();

	/*. Encrypt the plaintext*/
	retval = cmox_cipher_encrypt(CMOX_AES_CBC_ENC_ALGO,
			                    padded_plain_text, padded_len,
								Key,sizeof(Key),
								IV,sizeof(IV),
								cipher_text_bytes, &computed_size);

	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		handle_error("Encryption failed!");

	}


	/*. Decrypt the bytes*/
	retval = cmox_cipher_decrypt(CMOX_AES_CBC_DEC_ALGO,
			                     cipher_text_bytes, computed_size,
								Key,sizeof(Key),
								IV,sizeof(IV),
								decrypted_bytes, &computed_size);
	if(retval !=  CMOX_CIPHER_SUCCESS)
	{
		handle_error("Decryption failed!");

	}

	/*Remove padding from encryped buytes*/
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

