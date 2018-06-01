/* SMC Alert - Performance Tests
 * Marcus Botacin - UFPR - 2018
 */

/* Import Block */
#include<linux/init.h>		
#include<linux/module.h>	
#include<linux/fs.h>		
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>		
#include<linux/device.h>

/* Device Name */
#define DEVICE_NAME "smcmod"

/* PID Size */
#define PID_SIZE 5

/* Flag Size */
#define FLAG_SIZE 3

/* Device Object */
struct cdev *mcdev;		
/* Device Numbers */
dev_t dev_num;
int major_number, minor_number;

/* Device Class */
static struct class *cl; 

/* Device Structure */
struct _st_smc_device
{
    /* Interrupting process PID */
    char interrupted_process[PID_SIZE];
    /* Whitelist/Blacklist flag */
    char av_decision[FLAG_SIZE];
} smc_device;

/* Open Routine */
int device_open(struct inode *inode, struct file *filp) {
    /* Simulate a process interrupting the execution */
    strcpy(smc_device.interrupted_process,"123");
    return 0;
}

/* Close Routine */
int device_close(struct inode *inode, struct file *filp) 
{
    return 0;
}

/* Read Routine */
ssize_t device_read(struct file *filp, char *bufStoreData,
        size_t bufCount, loff_t* curOffset) 
{
    /* Tell the AV which process interrupted the execution */
    int ret = copy_to_user(bufStoreData, smc_device.interrupted_process, bufCount);
    return ret;
}

/* Write Routine */
ssize_t device_write(struct file *filp, const char *bufSourceData, 
        size_t bufCount, loff_t* curOffset)
{
    /* Receive AV decision to whitelist the process or not */
    int ret = copy_from_user(smc_device.av_decision, bufSourceData, bufCount);
    return ret;
}

/* Supported device operations */
struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = device_open,        /* Open the device */
    .release = device_close,    /* Close the device */
    .write = device_write,      /* Write into the device */
    .read = device_read         /* Read from the device */
};

/* Driver Initialization */
static int driver_entry(void) {
    
    /* char device region */
    int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    /* check errors */
    if(ret<0) {
        return ret;			
    }

    major_number = MAJOR(dev_num);
    minor_number = MINOR(dev_num);

    mcdev = cdev_alloc();		
    mcdev->ops = &fops;		
    mcdev->owner = THIS_MODULE;	

    /* Crate class */
    if ((cl = class_create(THIS_MODULE, "chardev")) == NULL)
    {
        /* On error cases */
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    /* Create device */
    if (device_create(cl, NULL, dev_num, NULL, DEVICE_NAME) == NULL)
    {
        /* on error, destroy */
        class_destroy(cl);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    /* Add device */
    ret = cdev_add(mcdev, dev_num, 1);
    /* check errors */
    if(ret<0) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    return 0;
}

/* Exit/Removal Routine */
static void driver_exit(void) {
    /* device delition*/
    cdev_del(mcdev);
    /* device destruction */
    device_destroy(cl, dev_num);	
    /* delete FS link */
    class_destroy(cl);		
    /* device unregister */
    unregister_chrdev_region(dev_num, 1);
}

/* Driver Entry */
module_init(driver_entry);	

/* Driver Exit */
module_exit(driver_exit); 

/* Module Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcus Botacin");
MODULE_DESCRIPTION("SMC Alert Modelling");
