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
	for (int i=0; i<SHA256_MSG_SIZE; i++) {
		msg[i] = 'A'+(i%26);
		printf("%c",msg[i]);
	}
  printf("\n\n");

   int ret, fd;
   printf("Starting device test code example...\n");
   fd = open("/dev/wssha256char", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   printf("Writing message to the device [%s].\n", msg);
   ret = write(fd, msg, strlen(msg)); // Send the string to the LKM
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }

	 printf("Sleeping for 2 seconds...\n");
	 sleep(2); 

   printf("Reading back from the device...\n");
   ret = read(fd, digest, SHA256_DGST_SIZE);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }

   printf("The received message is: [%s]\n", digest);
   printf("End of the program\n");
   return 0;
}
