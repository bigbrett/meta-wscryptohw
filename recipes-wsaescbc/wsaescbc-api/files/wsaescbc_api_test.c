#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "wsaescbc.h"



int main (void)
{

	const uint8_t key[AESKEYSIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
	const uint8_t iv[AESIVSIZE] =   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	const char openSSL_result[32] = { 0xA8, 0x87, 0x01, 0xE4, 0x43, 0x4F, 0x59, 0x00, 
		0x9F, 0xF8, 0x9A, 0x40, 0x29, 0x98, 0x49, 0x57,
		0x99, 0x29, 0x0C, 0x6C, 0xB1, 0xB1, 0x6D, 0x1A, 
		0x8B, 0x0A, 0xF7, 0xAF, 0x2D, 0x96, 0x7E, 0xF1 };



	//uint32_t len = 32;
    //uint32_t olen = 48; 
	//char teststr[] = "The Quick Brown Fox Jumped Over "; // string to encrypt
	char* teststr = "The Quick Brown Fox Jumped Over"; // string to encrypt
    
    uint32_t len = strlen(teststr); 
    uint32_t olen = (len < AESBLKSIZE) ? len : len + (AESBLKSIZE - (len % AESBLKSIZE));

	char buf0[olen];
	char buf1[olen];

	// zero out result array
	memset(buf0,0,olen);
	memset(buf1,0,olen);

	int ret = aes256init();
	if (0 != ret)
	{
		perror("ERROR in aes256init()");
		fprintf(stderr,"returned with exit code %d\n",errno);
		exit(1);
	}

	ret = aes256setiv((uint8_t*)iv);
	if (0 != ret)
	{
		perror("ERROR in aes256setiv()");
		fprintf(stderr,"returned with exit code %d\n",errno);
		exit(1);
	}

	ret = aes256setkey((uint8_t*)key);
	if (0 != ret) {
		perror("ERROR in aes256setkey()");
		fprintf(stderr,"returned with exit code %d\n",errno);
		exit(1);
	}

    // display input
    printf("ORIG LENGTHS: len = %d, olen = %d\n",len,olen);
    printf("\nInput:  ");
    for (int i=0; i<len; i++)
        printf("%c",teststr[i]);
    printf("\n");

	// encrypt
    printf("Encrypting.....\n");
	ret = aes256(ENCRYPT, (uint8_t*)teststr, len, (uint8_t*)buf0, &olen);
    printf("\tinlen = %d,   olen = %d\n",len,olen);
    printf("Ciphertext:  ");
    for (int i=0; i<olen; i++)
        printf("0x%02X ",buf0[i]);
    printf("\n");

    // decrypt
    printf("Decrypting.....\n");
	aes256(DECRYPT, (uint8_t*)buf0, olen, (uint8_t*)buf1, &olen);
    printf("\tinlen = %d,   olen = %d\n",len,olen);
    printf("Decrypted text: [ ");
    for (int i=0; i<len; i++)
        printf("%c",buf1[i]);
    for (int i=len; i<olen; i++)
        printf(", 0x%02X",buf1[i]);
    printf(" ]\n");

    printf("Checking Decryption.....\n");
	if (memcmp(buf1, teststr, len) != 0)
	{
		printf("ERROR: invalid Decryption\n");
		return -1;
	}
    printf("\tDecryption Success!\n");
    
	return 0;
}


