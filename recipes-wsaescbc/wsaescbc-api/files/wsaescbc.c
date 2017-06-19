/**
 * @file   testwsaescbckern.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the wsaescbckern.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/wsaeschar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "wsaescbc.h"
#include "wsaeskern.h"

static const char *devicefname = "/dev/wsaeschar";

/*
 *
 */
int32_t aes256init(void)
{
    printf("Checking for kernel module...\n");
    if( access( devicefname, F_OK ) != -1 ) 
    {
        printf("Found device!\n");
        return 0;
    } 
    else 
    {
        fprintf(stderr, "ERROR: Couldn't find device %s\n", devicefname);
        return -1; 
    }
}


/*
 *
 */
int32_t aes256setkey(uint8_t *keyp)
{
    int fd, ret = 0;
    
    // Open the device with read/write access
    fd = open("/dev/wsaeschar", O_RDWR);             
    if (fd < 0){
        perror("ERROR: Failed to open the device...");
        return errno;
    }

    printf("setting key\n");
    ret = ioctl(fd, IOCTL_SET_MODE, SET_KEY); // switch mode 
    if (ret < 0) {
        perror("Failed to set mode.");
        return errno;
    }
    ret = write(fd, keyp, AESKEYSIZE); // write key 
    if (ret < 0) {
        perror("Failed to write KEY to the device.");
        return errno;
    }

    // close and exit
    if(close(fd)<0)
        perror("aescbc: Error closing file");

    return 0;
}


/*
 *
 */
int32_t aes256setiv(uint8_t *ivp)
{
    int fd, ret = 0;
    // Open the device with read/write access
    fd = open("/dev/wsaeschar", O_RDWR);             
    if (fd < 0){
        perror("ERROR: Failed to open the device...");
        return errno;
    }

    printf("setting IV\n");
    ret = ioctl(fd, IOCTL_SET_MODE, SET_IV); // switch mode 
    if (ret < 0) {
        perror("Failed to set mode.");
        return errno;
    }
    ret = write(fd, ivp, AESIVSIZE); // write IV
    if (ret < 0) {
        perror("Failed to write IV to the device.");
        return errno;
    }
    
    // close and exit
    if(close(fd)<0)
        perror("aescbc: Error closing file");

    return 0;
}


/*
 * 
 */
int32_t aes256(int mode, uint8_t *inp, uint32_t inlen, uint8_t *outp, uint32_t *lenp) 
{
    int32_t fd, ret;
    int32_t len = *outp;

    // initialize output to all zeros
    memset((void*)outp,0,len);

    // check bounds against max length 
    if (inlen > AESMAXDATASIZE || len > AESMAXDATASIZE || inlen!=*lenp)
    {
        fprintf(stderr, "ERROR: Provided data length too large, must be less than %d bytes\n",
                AESMAXDATASIZE);
        return -1;
    }

    // Open the device with read/write access
    fd = open("/dev/wsaeschar", O_RDWR);             
    if (fd < 0){
        perror("ERROR: Failed to open the device...");
        return errno;
    }

    // Reset block 
    ret = ioctl(fd, IOCTL_SET_MODE, RESET); 
    if (ret < 0) {
        perror("ERROR: failed to reset AES block... \n");
        return errno;
    }

    // Set mode to ENCRYPT/DECRYPT
    if (mode != ENCRYPT && mode != DECRYPT)
    {
        fprintf(stderr, "ERROR: invalid mode. Must be either ENCRYPT or DECRYPT\n");
        return -1;
    }
    else
    {
        ret = ioctl(fd, IOCTL_SET_MODE, (ciphermode_t)mode); 
        if (ret < 0) {
            perror("ERROR: failed to set mode, ioctl returns errno \n");
            return errno;
        }
    }


    // send the bytes in 16-byte blocks to the LKM for encryption
    for (int i=0; i<inlen; i+=AESBLKSIZE)
    {
        printf("i=%d\n",i);

        // send 16 byte chunk from caller to AES block
        ret = write(fd, &(inp[i]), AESBLKSIZE); 
        if (ret < 0) {
            perror("ERROR: Failed to write data to the AES block... ");   
            return errno;                                                      
        }

        // read back processed 16 byte chunk into caller memory from AES block
        ret = read(fd, &(outp[i]), AESBLKSIZE);
        if (ret < 0){
            perror("Failed to read data back from the AES block... ");
            return errno;
        }
    }    

    // close and exit
    if(close(fd)<0)
    {
        perror("aescbc: Error closing file");
        return errno;
    }

    return 0;
}

