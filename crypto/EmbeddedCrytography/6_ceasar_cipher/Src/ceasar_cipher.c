#include "ceasar_cipher.h"



void decrypt_ceasar_cipher(uint8_t *encryptedtext, uint8_t shift, uint8_t *decryptedtext, uint32_t length)
{
	/*Declare variables for loop iterator and storing character*/
	uint32_t i;
	uint8_t c;

	/*Loop through each character of the encrypted text*/
	for( i = 0 ; i < length; i++)
	{
		/*Get character at index i*/
		 c  = encryptedtext[i];

		 /*Check if character is a lowercase letter*/
		 if(c >= 'a' && c <= 'z')
		 {
			 /*Decrypt lowercase letters, +26 ensures modulo operation is positive*/
			 c = (c - 'a' - shift +26) % 26 + 'a';
		 }

		 /*Check if character is a uppercase letter*/
		 else if(c >= 'A' && c <= 'Z')
		 {
			 /*Decrypt uppercase letters, +26 ensures modulo operation is positive*/

			 c = (c - 'A' - shift +26) % 26 + 'A';

		 }

		 decryptedtext[i] = c;
	}

	decryptedtext[length] = '\0';

}

void encrypt_ceasar_cipher(uint8_t *plaintext, uint8_t shift, uint8_t *encryptedtext, uint32_t length)
{
	/*Declare variables for loop iterator and storing character*/
	uint32_t i;
	uint8_t c;

	/*Loop through each character of the plaintext*/
	for( i = 0 ; i < length; i++)
	{
		/*Get character at index i*/
		 c  = plaintext[i];

		 /*Check if character is a lowercase letter*/
		 if(c >= 'a' && c <= 'z')
		 {
			 /*Encrypt lowercase letter*/
			 c = (c - 'a' + shift)% 26 +'a';
		 }

		 /*Check if character is a uppercase letter*/
		 else if(c >= 'A' && c <= 'Z')
		 {
			 /*Encrypt uppercase letter*/
			 c = (c - 'A' + shift)% 26 +'A';
		 }

		 /*Store the encrypted character*/
		 encryptedtext[i] =  c;

	}
	/*Null terminate the encrypted string*/
	encryptedtext[length]='\0';
}


uint16_t encrypt_ceasar_cipher_u16(uint16_t value, uint16_t shift)
{
	return(value +  shift) & 0xFFFF; /*& 0xFFFF ensures the result stays within 16-bits*/
}


uint16_t decrypt_ceasar_cipher_u16(uint16_t encrypted_value, uint16_t shift)
{
	return(encrypted_value -  shift) & 0xFFFF; /*& 0xFFFF ensures the result stays within 16-bits*/
}
