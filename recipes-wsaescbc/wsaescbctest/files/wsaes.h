/*
 * wsaes.h
 *
 *  Created on: Jun 6, 2017
 *      Author: brett
 */

#ifndef SRC_WSAES_H_
#define SRC_WSAES_H_

/* AES-256 */
#define AESKEYSIZE 32/* 256bits=32bytes */
#define AESIVSIZE 16 /* 128bits=16bytes */
#define AESBLKSIZE 16 /* 128bits=16bytes */

#define DATA_IN_SIZE 64

typedef enum { RESET = 0, ENCRYPT, DECRYPT, SET_IV, SET_KEY } ciphermode_t;

//int32_t aes256setkey(uint8_t *keyp);
//int32_t aes256setiv(uint8_t *ivp);
//int32_t aes256(int mode,uint8_t *inp,uint32_t inlen,uint8_t *outp,uint32_t *outlenp);
//int32_t aes256_init(void);
//static void startwait(void);
//static void dumpmsg( uint8_t *pbuf ) {

#endif /* SRC_WSAES_H_ */
