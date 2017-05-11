/*
 * sha256test.c
 *
 *  Created on: Apr 7, 2017
 *      Author: brett
 */


#include <stdio.h>
#include <stdint.h>
#include "wssha.h" // Device driver for HLS HW block

#define DATASIZE 64
#define SHA256_HASHSIZE 32

	uint8_t data[DATASIZE];                    // the data to hash
	volatile uint8_t digest[SHA256_HASHSIZE];  // the location of the digest
	uint32_t digest_len = SHA256_HASHSIZE;     // dummy variable will be overwritten

int main(void)
{
	// Fill data buffer with something interesting to hash
	for (int i=0; i<DATASIZE; i++) {
		data[i] = 'A'+(i%26);
		printf("%c",data[i]);
	}

	printf("\r\nTesting SHA256...\r\n");

	int32_t status = sha256(data, DATASIZE, (uint8_t*)digest, &digest_len);

	printf("Result received.\n\r");
	printf("SHA256_HASH: ");
	for (int i=0; i<SHA256_HASHSIZE; i++) {
		printf("%02X ", digest[i]);
	}
	printf("\r\n");

	return 0;
}
