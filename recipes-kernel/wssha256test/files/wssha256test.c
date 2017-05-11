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


#define XSHA256_AXILITES_ADDR_AP_CTRL          0x000
#define XSHA256_AXILITES_ADDR_GIE              0x004
#define XSHA256_AXILITES_ADDR_IER              0x008
#define XSHA256_AXILITES_ADDR_ISR              0x00c
#define XSHA256_AXILITES_ADDR_BASE_OFFSET_DATA 0x200
#define XSHA256_AXILITES_BITS_BASE_OFFSET_DATA 32
#define XSHA256_AXILITES_ADDR_BYTES_DATA       0x208
#define XSHA256_AXILITES_BITS_BYTES_DATA       32
#define XSHA256_AXILITES_ADDR_DATA_BASE        0x100
#define XSHA256_AXILITES_ADDR_DATA_HIGH        0x1ff
#define XSHA256_AXILITES_WIDTH_DATA            8
#define XSHA256_AXILITES_DEPTH_DATA            256
#define XSHA256_AXILITES_ADDR_DIGEST_BASE      0x220
#define XSHA256_AXILITES_ADDR_DIGEST_HIGH      0x23f
#define XSHA256_AXILITES_WIDTH_DIGEST          8
#define XSHA256_AXILITES_DEPTH_DIGEST          32

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)

static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM
static char stringToSend[BUFFER_LENGTH] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse ac tellus id nibh fringilla sodales. Suspendisse congue erat nec aliquam finibus. Mauris porttitor dapibus malesuada. Duis ut sagittis odio. Sed finibus, eros sed posuere aenean suscipit.";

int main(){
   int ret, fd;
   printf("Starting device test code example...\n");
   fd = open("/dev/wssha256char", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Writing message to the device [%s].\n", stringToSend);
   ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }

   printf("Reading back from the device...\n");

   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }

   printf("The received message is: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
}
