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
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define SHA256_MSG_SIZE 256
#define SHA256_DGST_SIZE 32

static char digest[SHA256_DGST_SIZE];     ///< The receive buffer from the LKM
static char msg[SHA256_MSG_SIZE]; 

int main(){

  memset((void*)digest,0,SHA256_DGST_SIZE);

  // Fill data buffer with something interesting to hash
  printf("Data to hash:\n");
  for (int i=0; i<SHA256_MSG_SIZE; i++) {
    msg[i] = 'A'+(i%26);
  }

  int ret, fd;
  printf("Starting device test code example...\n");
  fd = open("/dev/wssha256char", O_RDWR);             // Open the device with read/write access
  if (fd < 0){
    perror("Failed to open the device...");
    return errno;
  }

  ret = write(fd, msg, strlen(msg)); // Send the string to the LKM
  if (ret < 0){
    perror("Failed to write the message to the device.");
    return errno;
  }

  ret = read(fd, digest, SHA256_DGST_SIZE);        // Read the response from the LKM
  if (ret < 0){
    perror("Failed to read the message from the device.");
    return errno;
  }

  if(close(fd)<0)
    perror("Error closing file");

  
  printf("*The received message is:\n");
  for (int i=0; i<SHA256_DGST_SIZE; i++)
    printf("%X ", digest[i]);
  printf("\n");
  return 0;
}
