#include "monoalphabetic_cipher.h"


#include <stdio.h>
#include <string.h>



void decrypt_monoalphabetic_cipher(char *ciphertext, char *key, char *decryptedtext)
{

	 char c;

	/*Loop over each character of the ciphertext*/
	for(int i = 0; i < strlen(ciphertext);i++)
	{
		c =  ciphertext[i];

		/*Decrypt lowercase letter*/
		if( c >= 'a' && c <= 'z')
		{

			/*Loop through key to find matching character*/
			  for( int j = 0; j < 26; j++)
			  {
				  if(key[j]  ==  c)
				  {
					  /*Decrypt and store in decryptedtext buffer*/
					  decryptedtext[i] = 'a' +j;

					  /*Break inner once match is found*/
					  break;
				  }
			  }

		}

		/*Decrypt uppercase letter*/
		else if( c >= 'A' && c <= 'Z')
		{

			/*Loop through key to find matching character*/
			  for( int j = 0; j < 26; j++)
			  {
				  if(key[j]  ==  c)
				  {
					  /*Decrypt and store in decryptedtext buffer*/
					  decryptedtext[i] = 'A' +j;

					  /*Break inner once match is found*/
					  break;
				  }
			  }

		}
		/*Store if neither lowercase nor uppercase*/
		else
		{
			decryptedtext[i] = c;
		}

	}

	decryptedtext[ strlen(ciphertext)] = '\0';
}


void encrypt_monoalphabetic_cipher(char *plaintext, char *key, char *encryptedtext)
{
	int i;
    char c;

	for( i = 0; i < strlen(plaintext); i++ )
	{
			c  = plaintext[i];


			/*Encrypt lowercase letter*/
			if( c >= 'a' && c <= 'z')
			{
				encryptedtext[i] =  key[c-'a'];
			}

			/*Encrypt uppercase letter*/

			else if( c >= 'A' && c <= 'Z')
			{
				encryptedtext[i] =  key[c-'A'];
			}

			/*Store if neither lowercase nor uppercase*/
			else
			{
				encryptedtext[i] = c;
			}

	}

	encryptedtext[strlen(plaintext)] = '\0';

}
