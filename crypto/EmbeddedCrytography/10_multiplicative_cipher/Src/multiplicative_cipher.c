#include <stdio.h>
#include <string.h>
#include "multiplicative_cipher.h"



void encryt_multiplicative_cipher(char *plaintext, int key, char * encrytedtext)
{
	int i;
	char c;

	for( i = 0; i < strlen(plaintext); i++)
	{
		c  =  plaintext[i];

		if( c >= 'A' &&  c <= 'Z' )
		{
			//E(x) =  (x * key) %26
			c = ((c - 'A')*key) % 26 + 'A';
			encrytedtext[i] =  c;

		}


		else if( c >= 'a' &&  c <= 'z' )
		{
			//E(x) =  (x * key) %26
			c = ((c - 'a')*key) % 26 + 'a';
			encrytedtext[i] =  c;

		}

		else
		{
			encrytedtext[i] =  c;
		}
	}

	encrytedtext[strlen(plaintext)] = '\0';
}


int mod_inverse(int a, int m)
{
	int i;

	for( i = 1; i < m; i++)
	{
		if((a*i) % m == 1)
		{
			return i;
		}
	}
	return -1;
}


void decryt_multiplicative_cipher(char *encrytedtext, int key, char * decrytedtext)
{
	int i;
	char c;
	int key_inv;

	key_inv =  mod_inverse(key, 26);

	if(key_inv == -1)
	{
		//Return error
		return;
	}

	for( i = 0; i < strlen(encrytedtext); i++)
	{
		c =  encrytedtext[i];

		if( c >= 'A' &&  c <= 'Z' )
		{
			//D(x) =  (x * key_inv) %26
			c = ((c - 'A')*key_inv) % 26 + 'A';
			decrytedtext[i] =  c;

		}

		else if( c >= 'a' &&  c <= 'z' )
		{
			//D(x) =  (x * key_inv) %26
			c = ((c - 'a')*key_inv) % 26 + 'a';
			decrytedtext[i] =  c;

		}
		else
		{
			decrytedtext[i] =  c;

		}
	}

	decrytedtext[strlen(encrytedtext)] =  '\0';
}



