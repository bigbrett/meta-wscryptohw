/**
 * @file   wsaes.c
 * @author Brett Nicholas
 * @date   5/11/17
 * @version 0.1
 * @brief   
 * Linux loadable kernel module (LKM) for AES-CBC acceleator. This module maps to /dev/wsaes and
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
#include "wsaeskern.h" 						// ioctl numbers defined here

// TODO this needs to be changed...we should not be hardcoding these
#define WSAESBASEADDR 0x43C10000
#define XAESCBC_AXILITES_ADDR_AP_CTRL       0x00
#define XAESCBC_AXILITES_ADDR_GIE           0x04
#define XAESCBC_AXILITES_ADDR_IER           0x08
#define XAESCBC_AXILITES_ADDR_ISR           0x0c
#define XAESCBC_AXILITES_ADDR_MODE_DATA     0x10
#define XAESCBC_AXILITES_BITS_MODE_DATA     3
#define XAESCBC_AXILITES_ADDR_DATA_IN_BASE  0x20
#define XAESCBC_AXILITES_ADDR_DATA_IN_HIGH  0x3f
#define XAESCBC_AXILITES_WIDTH_DATA_IN      8
#define XAESCBC_AXILITES_DEPTH_DATA_IN      32
#define XAESCBC_AXILITES_ADDR_DATA_OUT_BASE 0x40
#define XAESCBC_AXILITES_ADDR_DATA_OUT_HIGH 0x4f
#define XAESCBC_AXILITES_WIDTH_DATA_OUT     8
#define XAESCBC_AXILITES_DEPTH_DATA_OUT     16

#define AESBLKSIZE 16 
#define AESKEYSIZE 32 

#define  DEVICE_NAME "wsaeschar"    ///< The device will appear at /dev/wsaes using this value
#define  CLASS_NAME  "wsaes"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Brett Nicholas");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for wsaes block in hardware");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static void __iomem *vbaseaddr = NULL;          // void pointer to virtual memory mapped address for the device

// Operation mode of the aes block
typedef enum { RESET = 0, ENCRYPT, DECRYPT, SET_IV, SET_KEY } ciphermode_t;
static ciphermode_t mode = RESET;

static volatile char data_in[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // Memory for bytes  passed from userspace
static volatile char data_out[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // Memory for bytes passed back to userspace
volatile static int numberOpens = 0;              // Counts the number of times the device is opened
static struct class*  wsaescharClass  = NULL; /// The device-driver class struct pointer
static struct device* wsaescharDevice = NULL; /// The device-driver device struct pointer


// The prototype functions for the character driver -- must come before the struct definition
static int     wsaes_open(struct inode *, struct file *);
static int     wsaes_release(struct inode *, struct file *);
static ssize_t wsaes_read(struct file *, char *, size_t, loff_t *);
static ssize_t wsaes_write(struct file *, const char *, size_t, loff_t *);
static long    wsaes_ioctl(struct file *, unsigned int, unsigned long);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
  .open = wsaes_open,
  .read = wsaes_read,
  .write = wsaes_write,
  .release = wsaes_release,
  .unlocked_ioctl = wsaes_ioctl
};


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init wsaes_init(void)
{
  printk(KERN_INFO "wsaes: Initializing the wsaes LKM\n");

  // request physical memory for driver 
  if (!request_mem_region(WSAESBASEADDR, SZ_64K, "wsaes")) {
    printk(KERN_ALERT "wsaes failed to request memory region\n");
    return -EBUSY;
  }
  // map reserved physical memory into into virtual memory TODO dtc support
  vbaseaddr = ioremap(WSAESBASEADDR, SZ_64K);
  if (! vbaseaddr) {
    printk(KERN_ALERT "wsaes unable to map virual memory\n");
    release_mem_region(WSAESBASEADDR, SZ_64K);
    return -EBUSY;
  }
  vbaseaddr = ioremap(WSAESBASEADDR, SZ_64K);
  printk(KERN_INFO "wsaes: Virtual Address = 0x%X\n",vbaseaddr);

  // Register the device class
  wsaescharClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(wsaescharClass)) {              // Check for error and clean up if there is
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_ALERT "wsaes: Failed to register device class\n");
    return PTR_ERR(wsaescharClass);          // Correct way to return an error on a pointer
  }
  printk(KERN_INFO "wsaes: device class registered correctly\n");

  // Register the device driver
  wsaescharDevice = device_create(wsaescharClass, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_NAME);
  if (IS_ERR(wsaescharDevice)) {             // Clean up if there is an error
    class_destroy(wsaescharClass);           // Repeated code but the alternative is goto statements
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_ALERT "wsaes: Failed to create the device\n");
    return PTR_ERR(wsaescharDevice);
  }
  printk(KERN_INFO "wsaes: device class created correctly\n"); // Made it! device was initialized

  // init hardware parameters 
  printk(KERN_INFO "wsaes: initializing wsaes block to mode RESET\n");
  mode = RESET;
  iowrite8(mode, vbaseaddr + XAESCBC_AXILITES_ADDR_MODE_DATA); // write new mode value to memory 
  return 0;
}


/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit wsaes_exit(void) 
{
  iounmap(vbaseaddr); // unmap device IO memory 
  release_mem_region(WSAESBASEADDR, SZ_64K);
  device_destroy(wsaescharClass, MKDEV(MAJOR_NUM, 0));     // remove the device
  class_unregister(wsaescharClass);                          // unregister the device class
  class_destroy(wsaescharClass);                             // remove the device class
  unregister_chrdev(MAJOR_NUM, DEVICE_NAME);              // unregister the major number
  printk(KERN_INFO "wsaes: Exiting\n");
}


/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int wsaes_open(struct inode *inodep, struct file *filep){
  if (numberOpens > 0)
  {
    printk(KERN_INFO "wsaes: Error: device already open\n");
    return -EBUSY;
  }
  numberOpens++;
  printk(KERN_INFO "wsaes: Device has been opened %d time(s)\n", numberOpens);
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
static ssize_t wsaes_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
  // Read data_out from PL  
  memcpy_fromio(data_out, vbaseaddr+XAESCBC_AXILITES_ADDR_DATA_OUT_BASE, AESBLKSIZE);

  // Copy data_out back into userspace (*to,*from,size)
  copy_to_user(buffer, data_out, AESBLKSIZE);

  printk(KERN_INFO "wsaes: Copied data of length %d bytes back to userspace\n", AESBLKSIZE);
  return AESBLKSIZE;  // clear the position to the start and return 0
}


/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the copy_from_user() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t wsaes_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{  
  // copy data from userspace into kernel message buffer
  copy_from_user(data_in, buffer, len);

  // write data from kernel message buffer to PL data_in register region
  memcpy_toio(vbaseaddr+XAESCBC_AXILITES_ADDR_DATA_IN_BASE, data_in, len);

  // start AES block using read-modify-write on ap_ctrl register
  unsigned int ctrl_reg = ioread32(vbaseaddr + XAESCBC_AXILITES_ADDR_AP_CTRL) & 0x80; // read and get bit
  iowrite32(ctrl_reg | 0x01, vbaseaddr + XAESCBC_AXILITES_ADDR_AP_CTRL);

  printk(KERN_INFO "wsaes: Received message of length %zu bytes from userspace\n", len);
  return len;
}



/* This function is called whenever a process tries to 
 * do an ioctl on our device file. We get two extra 
 * parameters (additional to the inode and file 
 * structures, which all device functions get): the number
 * of the ioctl called and the parameter given to the 
 * ioctl function.
 *
 * If the ioctl is write or read/write (meaning output 
 * is returned to the calling process), the ioctl call 
 * returns the output of this function.
 */
static long wsaes_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  int retval = 0;

  // Switch according to the ioctl called 
  switch (ioctl_num) 
  {
    case IOCTL_SET_MODE:
      
      // check mode is valid
      if ( (mode >= 0) && (mode <= 4)) 
      {
        printk(KERN_INFO "IOCTL_SET_MODE: mode = (%d)\n", mode);
        mode = (ciphermode_t)ioctl_param; // Get mode parameter passed to ioctl by user 
        iowrite8(mode, vbaseaddr + XAESCBC_AXILITES_ADDR_MODE_DATA); // write new mode value to memory 
	
	// if ioctl call was RESET, manually start the block since there will be no impending write to 
	// to start the block
	if (mode == RESET) {
  	  unsigned int ctrl_reg = ioread32(vbaseaddr + XAESCBC_AXILITES_ADDR_AP_CTRL) & 0x80; // read and get bit
  	  iowrite32(ctrl_reg | 0x01, vbaseaddr + XAESCBC_AXILITES_ADDR_AP_CTRL);
	}
      }
      // invalid argument 
      else  {
        printk(KERN_INFO "IOCTL_SET_MODE: INVALID MODE\n");
        retval = -EINVAL; 
      }
      break;

    case IOCTL_GET_MODE:
      printk(KERN_INFO "IOCTL_GET_MODE: mode = (%d)\n", mode);
//      retval = put_user(mode, (ciphermode_t*)ioctl_param); // copy mode value back to userspace pointer
      break;

      // improper ioctl number, return error
    default:
      printk(KERN_INFO "ERROR, IMPROMER IOCTL NUMBER <%d>\n" ioctl_num);
      retval = -ENOTTY;
      break;
  }
  return retval;
}


/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int wsaes_release(struct inode *inodep, struct file *filep)
{
  printk(KERN_INFO "wsaes: Device successfully closed\n");
  numberOpens--;
  return 0;
}



/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(wsaes_init);
module_exit(wsaes_exit);
