#ifndef VIGENERE_CIPHER_H
#define VIGENERE_CIPHER_H


void encrypt_vigenere_cipher(char *plaintext, char *keyword, char *encryptedtext);
void decrypt_vigenere_cipher(char *encryptedtext, char *keyword, char *decryptedtext);
void decrypt_vigenere_cipher2(char *encryptedtext, char *keyword, char *decryptedtext);
void encrypt_vigenere_cipher2(char *plaintext, char *keyword, char *encryptedtext);

#endif
