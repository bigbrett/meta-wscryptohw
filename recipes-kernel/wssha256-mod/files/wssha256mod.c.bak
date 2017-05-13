/**
 * @file   wssha256.c
 * @author Brett Nicholas
 * @date   5/11/17
 * @version 0.1
 * @brief   
 * Linux loadable kernel module (LKM) for sha256 acceleator. This module maps to /dev/wssha256 and
 * comes with a helper C program that can be run in Linux user space to communicate with this LKM.
 * 
 * Based on the BBB LED driver template by Derek Molloy
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#include <linux/io.h>

// xilinx helper functions 
#include <xsha256.h>

// TODO this needs to be changed...we should not be hardcoding these
#define WSSHA256BASEADDR 0x43C00000
#define SHA256_DGST_SIZE 32 
#define SHA256_MSG_SIZE  256

// IOCTL modes to write 
//#define IOCTL_SET_DIGEST _IOW(MAJOR_NUM,  0, uint32_t)
//#define IOCTL_SET_MSG    _IOW(MAJOR_NUM,  1, uint32_t)
//#define IOCTL_START      _IOW(MAJOR_NUM,  2, uint32_t)

#define  DEVICE_NAME "wssha256char"    ///< The device will appear at /dev/wssha256 using this value
#define  CLASS_NAME  "wssha256"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Brett Nicholas");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for wssha256 block in hardware");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int majorNumber;                  ///< Stores the device number -- determined automatically
static void __iomem *vbaseaddr;          // void pointer to virtual memory mapped address for the device
static volatile char message[SHA256_MSG_SIZE] = {0}; ///< Memory for the string (message)that is passed from userspace
static volatile char digest[SHA256_DGST_SIZE] = {0}; ///< Memory for the string (digest) that is passed back to userspace
static short size_of_message;              ///< Used to remember the size of the string stored
volatile static int numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  wssha256charClass  = NULL; ///< The device-driver class struct pointer
static struct device* wssha256charDevice = NULL; ///< The device-driver device struct pointer

static XSha256 XShaInst; // instance pointer to use xilinx helper funcitons 


// The prototype functions for the character driver -- must come before the struct definition
static int     wssha256_open(struct inode *, struct file *);
static int     wssha256_release(struct inode *, struct file *);
static ssize_t wssha256_read(struct file *, char *, size_t, loff_t *);
static ssize_t wssha256_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.open = wssha256_open,
	.read = wssha256_read,
	.write = wssha256_write,
	.release = wssha256_release,
};


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init wssha256_init(void){
	printk(KERN_INFO "wssha256: Initializing the wssha256 LKM\n");

	// map device into memory and initialize structure
	// TODO dtc support
	vbaseaddr = ioremap(WSSHA256BASEADDR, SZ_64K);
	XShaInst.Axilites_BaseAddress = vbaseaddr;
	XShaInst.IsReady = 1;

	printk(KERN_INFO "\tphysical address = 0x%X\nvirtual address = 0x%X\n", WSSHA256BASEADDR, XShaInst.Axilites_BaseAddress);

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "wssha256 failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "wssha256: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	wssha256charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(wssha256charClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(wssha256charClass);          // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "wssha256: device class registered correctly\n");

	// Register the device driver
	wssha256charDevice = device_create(wssha256charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(wssha256charDevice)){               // Clean up if there is an error
		class_destroy(wssha256charClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(wssha256charDevice);
	}
	printk(KERN_INFO "wssha256: device class created correctly\n"); // Made it! device was initialized
	return 0;
}


/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit wssha256_exit(void){
	iounmap(vbaseaddr); // unmap device IO memory 
	device_destroy(wssha256charClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(wssha256charClass);                          // unregister the device class
	class_destroy(wssha256charClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);              // unregister the major number
	printk(KERN_INFO "wssha256: Exiting\n");
}


/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int wssha256_open(struct inode *inodep, struct file *filep){
	//if (numberOpens > 0)
	//{
	//  printk(KERN_INFO "Error: device already open\n");
	//  return -EBUSY;
	//}
	numberOpens++;
	printk(KERN_INFO "wssha256: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}


/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the buffer
 *  @param offset The offset if required
 */
static ssize_t wssha256_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
	int error_count = 0;
	printk(KERN_INFO "executing copy to user\n");
	// Read digest from hardware 
  XSha256_Read_digest_Bytes(&XShaInst, 0, &digest, SHA256_DGST_SIZE);
  // Copy digest back into userspace
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error_count = copy_to_user(buffer, digest, SHA256_DGST_SIZE);

	if (error_count==0){            // if true then have success
		printk(KERN_INFO "wssha256: Sent %d characters to the user\n", SHA256_DGST_SIZE);
		return (size_of_message=0);  // clear the position to the start and return 0
	}
	else {
		printk(KERN_INFO "wssha256: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
}


/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the copy_from_user() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t wssha256_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	printk(KERN_INFO "executing copy from user\n");
	// copy_from_user has the format ( * to, *from, size) and returns 0 on success
	copy_from_user(&message, buffer, len);
  // write data into message memory area
  XSha256_Write_data_Words(&XShaInst, 0, &message, SHA256_MSG_SIZE);
	size_of_message = strlen(message);                 // store the length of the stored message
	printk(KERN_INFO "wssha256: Received %zu characters from the user\n", len);
	return len;
}


/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int wssha256_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "wssha256: Device successfully closed\n");
	return 0;
}


//long wssha256_ioctl(struct file *filep, unsigned int ioctl_num, unsigned long ioctl_param)
//{
//	switch(ioctl_num) 
//  {
//		case IOCTL_SET_DIGEST:
//			printk(KERN_INFO "IOCTL set digest\n");
//			break;
//
//		case IOCTL_SET_MSG:
//			printk(KERN_INFO "IOCTL set msg\n");
//			break;
//
//		case IOCTL_SET_COLPD:
//			printk(KERN_INFO "IOCTL start\n");
//			break;
//	}
//	return 0;
//}


/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(wssha256_init);
module_exit(wssha256_exit);
