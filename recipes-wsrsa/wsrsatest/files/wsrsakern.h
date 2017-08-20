/**
 * @file   wsrsa.h
 * @author Brett Nicholas
 * @date   5/11/17
 * @version 0.1
 * @brief   
 * Header file for a Linux loadable kernel module (LKM) for an RSA acceleator. This 
 * module maps to /dev/wsrsa and comes with a helper C program that can be run in Linux user space 
 * to communicate with this LKM.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known BOTH to the kernel module
 *  (in wsrsa.c) and the userspace process calling ioctl (driver) 
 */

#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>


/*
 * These are the modes that the module can be in, set using IOCTL
 */
typedef enum {ENCRYPT=0, DECRYPT=1, SET_PKEY=2 } rsamode_t;

/*
 * Make the data structure holding public information accessible to caller
 */
typedef struct {
    char base[RSA_SIZE_BYTES];
    char exponent[RSA_SIZE_BYTES];
    char modulus[RSA_SIZE_BYTES];
} RSAPublic_t;

/* The major device number. We can't rely on dynamic 
 * registration any more, because ioctls need to know 
 * it. */
#define MAJOR_NUM 102

/* _IOR means that we're creating an ioctl command 
 * number for passing information from a user process
 * to the kernel module. We (unintuitively) use _IOR for 
 * IOCTL_GET_MODE because even though we want to "read"
 * the mode from the kernel module's register, we actually do  
 * this by passing a pointer from userspace into the module 
 *
 * The first arguments, MAJOR_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */
#define IOCTL_SET_MODE _IOR(MAJOR_NUM, 0, char) /* Set the message of the device driver */
#define IOCTL_GET_MODE _IOR(MAJOR_NUM, 1, char) /* Get the message of the device driver */
 
#endif
