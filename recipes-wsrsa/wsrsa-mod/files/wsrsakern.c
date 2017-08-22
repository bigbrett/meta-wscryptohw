/**
 * @file   wsrsa.c
 * @author Brett Nicholas
 * @date   5/11/17
 * @version 0.1
 *    
 * Linux loadable kernel module (LKM) for RSA- acceleator. This module maps to /dev/wsrsa and
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
#include <linux/sizes.h>
#include "wsrsakern.h" 						// ioctl numbers defined here

// These are pulled straight from the Vivado exported hardware
#define WSRSABASEADDR 0x43C00000 // mirrors XPAR_WSRSA1024_0_S_AXI_AXILITES_BASEADDR in xparameters.h
#define XWSRSA1024_AXILITES_ADDR_AP_CTRL         0x000
#define XWSRSA1024_AXILITES_ADDR_GIE             0x004
#define XWSRSA1024_AXILITES_ADDR_IER             0x008
#define XWSRSA1024_AXILITES_ADDR_ISR             0x00c
#define XWSRSA1024_AXILITES_ADDR_MODE_DATA       0x010
#define XWSRSA1024_AXILITES_BITS_MODE_DATA       2
#define XWSRSA1024_AXILITES_ADDR_BASE_V_DATA     0x018
#define XWSRSA1024_AXILITES_BITS_BASE_V_DATA     1024
#define XWSRSA1024_AXILITES_ADDR_BASE_V_DATA_    0x040
#define XWSRSA1024_AXILITES_BITS_BASE_V_DATA     1024
#define XWSRSA1024_AXILITES_ADDR_PUBLEXP_V_DATA  0x09c
#define XWSRSA1024_AXILITES_BITS_PUBLEXP_V_DATA  1024
#define XWSRSA1024_AXILITES_ADDR_PUBLEXP_V_DATA_ 0x0c4
#define XWSRSA1024_AXILITES_BITS_PUBLEXP_V_DATA  1024
#define XWSRSA1024_AXILITES_ADDR_MODULUS_V_DATA  0x120
#define XWSRSA1024_AXILITES_BITS_MODULUS_V_DATA  1024
#define XWSRSA1024_AXILITES_ADDR_MODULUS_V_DATA_ 0x148
#define XWSRSA1024_AXILITES_BITS_MODULUS_V_DATA  1024
#define XWSRSA1024_AXILITES_ADDR_RESULT_V_DATA   0x1a4
#define XWSRSA1024_AXILITES_BITS_RESULT_V_DATA   1024
#define XWSRSA1024_AXILITES_ADDR_RESULT_V_DATA_  0x1cc
#define XWSRSA1024_AXILITES_BITS_RESULT_V_DATA   1024


#define  DEVICE_NAME "wsrsachar"    ///< The device will appear at /dev/wsrsa using this value
#define  CLASS_NAME  "wsrsa"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Brett Nicholas");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for wsrsa block in hardware");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static void __iomem *vbaseaddr = NULL;          // void pointer to virtual memory mapped address for the device

// Operation mode of the rsa block
static rsamode_t mode = ENCRYPT;

volatile static int numberOpens = 0;          // Counts the number of times the device is opened
static struct class*  wsrsacharClass  = NULL; // The device-driver class struct pointer
static struct device* wsrsacharDevice = NULL; // The device-driver device struct pointer


// The prototype functions for the character driver -- must come before the struct definition
static int     wsrsa_open(struct inode *, struct file *);
static int     wsrsa_release(struct inode *, struct file *);
static ssize_t wsrsa_read(struct file *, char *, size_t, loff_t *);
static ssize_t wsrsa_write(struct file *, const char *, size_t, loff_t *);
static long    wsrsa_ioctl(struct file *, unsigned int, unsigned long);

// helper functions 
static void wsrsa_runonce_blocking(void);


/**  Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations 
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
    .open = wsrsa_open,
    .read = wsrsa_read,
    .write = wsrsa_write,
    .release = wsrsa_release,
    .unlocked_ioctl = wsrsa_ioctl
};


/**  The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init wsrsa_init(void)
{
    int ret = 0; 
    printk(KERN_INFO "wsrsa1024: Initializing the wsrsa LKM\n");

    // request physical memory for driver 
    if (!request_mem_region(WSRSABASEADDR, SZ_64K, "wsrsa")) {
        printk(KERN_ALERT "wsrsa failed to request memory region\n");
        return -EBUSY;
    }
    // map reserved physical memory into into virtual memory TODO dtc support
    vbaseaddr = ioremap(WSRSABASEADDR, SZ_64K);
    if (! vbaseaddr) {
        printk(KERN_ALERT "wsrsa unable to map virual memory\n");
        release_mem_region(WSRSABASEADDR, SZ_64K);
        return -EBUSY;
    }
    vbaseaddr = ioremap(WSRSABASEADDR, SZ_64K);
    printk(KERN_INFO "wsrsa1024: Virtual Address = 0x%X\n", (unsigned int)vbaseaddr);

    // Try to statically allocate a major number for the device driver
    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ALERT "wsrsa failed to register major number %d\n",MAJOR_NUM);
        return ret;
    }
    printk(KERN_INFO "wsrsa1024: registered correctly with major number %d\n", MAJOR_NUM);    

    // Register the device class with sysfs
    wsrsacharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(wsrsacharClass)) {              // Check for error and clean up if there is
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
        printk(KERN_ALERT "wsrsa1024: Failed to register device class\n");
        return PTR_ERR(wsrsacharClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "wsrsa1024: device class registered correctly\n");

    // Register the driver for the device class with sysfs
    wsrsacharDevice = device_create(wsrsacharClass, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_NAME);
    if (IS_ERR(wsrsacharDevice)) {             // Clean up if there is an error
        class_destroy(wsrsacharClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
        printk(KERN_ALERT "wsrsa1024: Failed to create the device\n");
        return PTR_ERR(wsrsacharDevice);
    }
    printk(KERN_INFO "wsrsa1024: device class created correctly\n"); // Made it! device was initialized

    // init mode to ENCRYPT
    printk(KERN_INFO "wsrsa1024: initializing wsrsa block to mode ENCRYPT\n");
    mode = ENCRYPT;
    iowrite8(mode, vbaseaddr + XWSRSA1024_AXILITES_ADDR_MODE_DATA); // write new mode value to memory 

    // Disable autorestart
    iowrite8(0, vbaseaddr + XWSRSA1024_AXILITES_ADDR_AP_CTRL);
    return 0;
}


/**  The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit wsrsa_exit(void) 
{
    iounmap(vbaseaddr); // unmap device IO memory 
    release_mem_region(WSRSABASEADDR, SZ_64K);
    device_destroy(wsrsacharClass, MKDEV(MAJOR_NUM, 0));     // remove the device
    class_unregister(wsrsacharClass);                          // unregister the device class
    class_destroy(wsrsacharClass);                             // remove the device class
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);              // unregister the major number
    printk(KERN_INFO "wsrsa1024: Exiting\n");
}


/**  The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  param:  inodep A pointer to an inode object (defined in linux/fs.h)
 *  param:  filep A pointer to a file object (defined in linux/fs.h)
 */
static int wsrsa_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "wsrsa1024: OPEN\n");
    if (numberOpens > 0)
    {
        printk(KERN_INFO "wsrsa1024: Error: device already open\n");
        return -EBUSY;
    }
    numberOpens++;
    printk(KERN_INFO "wsrsa1024: Device has been opened %d time(s)\n", numberOpens);
    return 0;
}


/**  This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  param:  filep A pointer to a file object (defined in linux/fs.h)
 *  param:  buffer The pointer to the buffer to which this function writes the data
 *  param:  len The length of the buffer
 *  param:  offset The offset if required
 */
static ssize_t wsrsa_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    unsigned int data_out[32]; // Memory for bytes passed back to userspace

    // copyt ciphertext/plaintext data_from AXI Memory to kmem
    //memcpy_fromio(data_out, vbaseaddr+XWSRSA1024_AXILITES_ADDR_RESULT_V_DATA, RSA_SIZE_BYTES);

    printk(KERN_INFO "RESULT = ");
    unsigned int *reg = vbaseaddr+XWSRSA1024_AXILITES_ADDR_RESULT_V_DATA;
    int i;
    for (i=0; i<32; i++)
    {
        data_out[i] = ioread32(reg++);
        printk(KERN_CONT "0x%08X, ",data_out[i]);
    }
    printk(KERN_INFO "\n");

    // Copy data_out from kmem into userspace (*to,*from,size)
    copy_to_user(buffer, data_out, RSA_SIZE_BYTES);

    printk(KERN_INFO "wsrsa1024: Copied data of length %d bytes back to userspace\n", RSA_SIZE_BYTES);
    return RSA_SIZE_BYTES;  
}


/**  This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the copy_from_user() function along with the length of the string.
 *  param:  filep A pointer to a file object
 *  param:  buffer The buffer to that contains the string to write to the device
 *  param:  len The length of the array of data that is being passed in the const char buffer
 *  param:  offset The offset if required
 */
static ssize_t wsrsa_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{  
    RSAPublic_t PublicData; // Memory for bytes passed from userspace

    // copy base,exponent,modulus from userspace-->kmem struct
    copy_from_user(&PublicData, buffer, sizeof(RSAPublic_t)); 
    print_hex_dump_bytes(".base    = ",0, PublicData.base, RSA_SIZE_BYTES);
    print_hex_dump_bytes(".exp     = ",0,PublicData.exponent,RSA_SIZE_BYTES);
    print_hex_dump_bytes(".modulus = ",0,PublicData.modulus,RSA_SIZE_BYTES);

    // copy base from kmem into AXI memory WORD AT A TIME 
    int byte_offset;
    //printk(KERN_INFO "BASE WRITTEN = ");
    //printk(KERN_INFO "BASE ADDRS = ");
    for (byte_offset=0; byte_offset<128; byte_offset+=4) 
    {
        iowrite32(*((unsigned int*)(PublicData.base + byte_offset)), vbaseaddr+XWSRSA1024_AXILITES_ADDR_BASE_V_DATA + byte_offset);     
        //printk(KERN_CONT "0x%08X, ",*((unsigned int*)(PublicData.base + byte_offset)));
        //printk(KERN_CONT "0x%X ", vbaseaddr+XWSRSA1024_AXILITES_ADDR_BASE_V_DATA + byte_offset);;    
    }
    printk(KERN_INFO "\n");

    // copy exponent from kmem into AXI memory WORD AT A TIME 
    //printk(KERN_INFO "EXP WRITTEN = ");
    for (byte_offset=0; byte_offset<128; byte_offset+=4) 
    {
        iowrite32(*((unsigned int*)(PublicData.exponent+ byte_offset)), vbaseaddr+XWSRSA1024_AXILITES_ADDR_PUBLEXP_V_DATA + byte_offset);     
        //printk(KERN_CONT "0x%08X, ",*((unsigned int*)(PublicData.exponent+ byte_offset)));
    }
    //printk(KERN_INFO "\n");

    // copy modulus from kmem into AXI memory WORD AT A TIME 
    //printk(KERN_INFO "MOD WRITTEN = ");
    for (byte_offset=0; byte_offset<128; byte_offset+=4) 
    {
        iowrite32(*((unsigned int*)(PublicData.modulus+ byte_offset)), vbaseaddr+XWSRSA1024_AXILITES_ADDR_MODULUS_V_DATA + byte_offset);     
        //printk(KERN_CONT "0x%08X, ",*((unsigned int*)(PublicData.modulus+ byte_offset)));
    }    
    //printk(KERN_INFO "\n");

    // start RSA block to encrypt/decrypt
    wsrsa_runonce_blocking();

    printk(KERN_INFO "wsrsa1024: Received message of length %zu bytes from userspace\n", len);
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
static long wsrsa_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    int retval = 0;

    // Switch according to the ioctl called 
    switch (ioctl_num) 
    {
        case IOCTL_SET_MODE:
            // check mode is valid
            if ( (mode >= 0) && (mode <= 2)) 
            {
                printk(KERN_INFO "IOCTL_SET_MODE: curr_mode = (%d)\n",mode);
                mode = (rsamode_t)ioctl_param; // Get mode parameter passed to ioctl by user 
                printk(KERN_INFO "new mode = (%d)\n", mode);
                iowrite8(mode, vbaseaddr + XWSRSA1024_AXILITES_ADDR_MODE_DATA); // write new mode value to memory 

                // If mode is SET_PRIVKEY, we must run the block once to load key from BRAM 
                if (mode == SET_PRIVKEY)
                    wsrsa_runonce_blocking();
            }
            // invalid argument 
            else  {
                printk(KERN_INFO "IOCTL_SET_MODE: INVALID MODE\n");
                retval = -EINVAL; 
            }
            break;

        case IOCTL_GET_MODE:
            printk(KERN_INFO "IOCTL_GET_MODE: mode = (%d)\n", mode);
            retval = put_user(mode, (rsamode_t*)ioctl_param); // copy mode value back to userspace pointer
            break;

            // improper ioctl number, return error
        default:
            printk(KERN_INFO "ERROR, IMPROPER IOCTL NUMBER <%d>\n", ioctl_num);
            retval = -ENOTTY;
            break;
    }
    return retval;
}


/**  The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  param:  inodep A pointer to an inode object (defined in linux/fs.h)
 *  param:  filep A pointer to a file object (defined in linux/fs.h)
 */
static int wsrsa_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "wsrsa1024: Device successfully closed\n");
    numberOpens--;
    return 0;
}


/*
 * Helper function to check if the block has finished and is ready for the next input
 */
static void wsrsa_runonce_blocking(void)
{
    unsigned int ctrl_reg;

    // set ap_start high using read-modify-write
    ctrl_reg = ioread32(vbaseaddr + XWSRSA1024_AXILITES_ADDR_AP_CTRL) & 0x80; 
    iowrite32(ctrl_reg | 0x01, vbaseaddr + XWSRSA1024_AXILITES_ADDR_AP_CTRL);

    // wait for completion
    while (! (ioread32(vbaseaddr + XWSRSA1024_AXILITES_ADDR_AP_CTRL)>>1) & 0x1); 
}


/**  A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(wsrsa_init);
module_exit(wsrsa_exit);
