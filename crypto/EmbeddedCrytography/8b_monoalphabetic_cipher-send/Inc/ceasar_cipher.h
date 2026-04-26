#ifndef CEASAR_CIPHER_H
#define CEASAR_CIPHER_H

#include <stdio.h>
#include <string.h>

void encrypt_ceasar_cipher(uint8_t *plaintext, uint8_t shift, uint8_t *encryptedtext, uint32_t length);
void decrypt_ceasar_cipher(uint8_t *encryptedtext, uint8_t shift, uint8_t *decryptedtext, uint32_t length);
uint16_t encrypt_ceasar_cipher_u16(uint16_t value, uint16_t shift);
uint16_t decrypt_ceasar_cipher_u16(uint16_t encrypted_value, uint16_t shift);
void decrypt_ceasar_hack_test(char encryptedtext[], uint8_t key);

#endif
