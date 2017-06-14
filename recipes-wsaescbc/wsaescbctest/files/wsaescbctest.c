/**
 * @file   testwssha256kern.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the wssha256kern.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/wssha256char.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "wsaes.h"
#include "wsaeskern.h" // IOCTL MACROS 


uint8_t key[AESKEYSIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
							              0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
							              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
							              0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
uint8_t iv[AESIVSIZE] =   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		 	 	   	   	            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
char openSSL_result[32] = { 0xA8, 0x87, 0x01, 0xE4, 0x43, 0x4F, 0x59, 0x00, 
                            0x9F, 0xF8, 0x9A, 0x40, 0x29, 0x98, 0x49, 0x57,
							              0x99, 0x29, 0x0C, 0x6C, 0xB1, 0xB1, 0x6D, 0x1A, 
                            0x8B, 0x0A, 0xF7, 0xAF, 0x2D, 0x96, 0x7E, 0xF1 };
const char teststr[32] = "The Quick Brown Fox Jumped Over "; // string to encrypt
const char* devstr = "/dev/wsaeschar";

//int32_t aes256setkey(int fd, uint8_t *keyp)
//{
//	return XST_SUCCESS;
//}
//
//
///*
// *
// */
//int32_t aes256setiv(uint8_t *ivp)
//{
//	return XST_SUCCESS;
//}
//
//
///*
// *
// */
//int32_t aes256_init(void)
//{
//	XAescbc_DisableAutoRestart(&xaescbc);
//	return XST_SUCCESS;
//}
//
//
///*
// *
// */
//int32_t aes256_reset(void)
//{
//	return XST_SUCCESS;
//}
//
//
///*
// *
// */
//int32_t aes256(int mode,uint8_t *inp,uint32_t inlen,uint8_t *outp,uint32_t *outlenp)
//{
//	return XST_SUCCESS;
//}
//
//
//
static void dumpmsg( uint8_t *pbuf ) {
	int index;

	for( index = 0; index < 16; index++ ) {
		printf("%02X ", pbuf[index]);
	}
	printf("\n");
}

int main (void)
{
  int ret, fd, errcnt;

  // Open the device with read/write access
	printf("Beginning AES test...\n");
  printf("Looking for <%s>\n",devstr);
  fd = open(devstr, O_RDWR);             
  if (fd < 0){
    perror("Failed to open the device, ERROR: ");
    return errno;
  }

  //errcnt=0;

  //// initialize input and output memory
	//uint8_t buf0[32];
	//uint8_t buf1[32];
	//memset(buf0,0,32);
	//memset(buf1,0,32);

	//// Text to encrypt/decrypt
	//strncpy((char *)buf0, teststr, 32);
	//printf("TEXT  :\n");
	//dumpmsg(buf0);
	//dumpmsg(&(buf0[16]));

	//// Initialize our engine
	////aes256_init();

  //// set key
  //ret = ioctl(fd, IOCTL_SET_MODE, SET_KEY); // switch mode 
	//ret = write(fd, key, AESKEYSIZE); // write key 
	//if (ret < 0) {
	//	perror("Failed to write KEY to the device.");
	//	return errno;
	//}

  //// set IV
  //ret = ioctl(fd, IOCTL_SET_MODE, SET_IV); // switch mode 
	//ret = write(fd, key, AESIVSIZE); // write IV
	//if (ret < 0) {
	//	perror("Failed to write IV to the device.");
	//	return errno;
	//}
 
  //// reset block 
  //ret = ioctl(fd, IOCTL_SET_MODE, RESET); 
	//if (ret < 0) {
	//	perror("ERROR: RESET IOCTL FAILED\n");
	//	return errno;
	//}
  //// Encrypt first 16 bytes
	//ret = ioctl(fd, IOCTL_SET_MODE, ENCRYPT); // switch mode 
	//ret = write(fd, buf0, AESBLKSIZE); 
	//if (ret < 0) {
	//	perror("Failed to write first block of data to the device.");
	//	return errno;
	//}

  //// read back encrypted first 16 bytes into buffer
	//ret = read(fd, buf1, AESBLKSIZE);        
	//if (ret < 0){
	//	perror("Failed to read first block of encrypted data from the device.");
	//	return errno;
	//}

  //// Encrypt second 16 bytes
	//ret = write(fd, &(buf0[16]), AESBLKSIZE); 
	//if (ret < 0) {
	//	perror("Failed to write second block of data to the device.");
	//	return errno;
	//}	

  //// read back encrypted second 16 bytes into buffer
	//ret = read(fd, &(buf1[16]), AESBLKSIZE);        
	//if (ret < 0){
	//	perror("Failed to read second block of encrypted data from the device.");
	//	return errno;
	//}

  //// display encrypted data
	//printf("DUTENC:\n");
	//dumpmsg(buf1);
	//dumpmsg(&(buf1[16]));

  //// Check encrypted data against ground truth
	//if (memcmp(buf1, openSSL_result, 32))
	//{
	//	printf("ERROR: ENCRYPTED DATA NOT CORRECT\n");
	//	return -1;
	//}

	//// Erase the original plain text
	//memset(buf0,0,32);

	//// Reset, set mode, and write first 16 bytes of DECRYPTED data to block
	//ret = ioctl(fd, IOCTL_SET_MODE, RESET);
	//if (ret < 0) {
	//	perror("ERROR: RESET IOCTL FAILED\n");
	//	return errno;
	//}

  //// Decrypt first 16 bytes
	//ret = ioctl(fd, IOCTL_SET_MODE, DECRYPT); // switch mode 
	//ret = write(fd, buf1, AESBLKSIZE); 
	//if (ret < 0) {
	//	perror("Failed to write first block of encrypted data to the device.");
	//	return errno;
	//}

  //// read back encrypted second 16 bytes into buffer
	//ret = read(fd, buf0, AESBLKSIZE);        
	//if (ret < 0){
	//	perror("Failed to read first block of decrypted data from the device.");
	//	return errno;
	//}

	//// Decrypt second 16 bytes
	//ret = write(fd, &(buf1[16]), AESBLKSIZE); 
	//if (ret < 0) {
	//	perror("Failed to write second block of encrypted data to the device.");
	//	return errno;
	//}

  //// read back decrypted second 16 bytes into buffer
	//ret = read(fd, &(buf0[16]), AESBLKSIZE);        
	//if (ret < 0){
	//	perror("Failed to read second block of decrypted data from the device.");
	//	return errno;
	//}

  //// Display decrypted data
	//printf("DUTDEC:\n");
	//dumpmsg(buf0);
	//dumpmsg(&(buf0[16]));

	//// close and exit
  //if(close(fd)<0)
  //  perror("Error closing file");

  //// check decrypted data against original message
	//if (memcmp(buf0, teststr, 32))
	//{
	//	printf("ERROR: DECRYPTED DATA NOT CORRECT\n");
	//	return -1;
	//}

	printf("Success!\n");
	return 0;
}


