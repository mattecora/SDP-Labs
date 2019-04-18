#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/cdev.h>
#include <linux/string.h>

/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "chardev_SDP_lab"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 128						/* Max length of the message from the device */

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;			            /* Major number assigned to our device driver */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

struct cdev *my_cdev;
dev_t dev_no, dev;

char char_dev_buf[BUF_LEN];
char *buf_ptr = char_dev_buf;

/* It was DECLARE_MUTEX(me), but this is not supported anymore */
static DEFINE_SEMAPHORE(me);	// mutex initialized

/*
 * This function is called when the module is loaded
 */
int 
init_module(void)
{
    unsigned int firstminor;
    unsigned int count;
    int ret;

    my_cdev = cdev_alloc();	
    my_cdev->ops = &fops;
    my_cdev->owner = THIS_MODULE;
    cdev_init(my_cdev, &fops);
  
    firstminor = 0;
    count = 1; // just one minor number
    ret = alloc_chrdev_region(&dev_no, firstminor, count, DEVICE_NAME); 
  
    if (ret < 0)
    {
        printk(KERN_ALERT "Registering char device failed with %d.\n", Major);
        return ret;
    }

    Major = MAJOR(dev_no);
    dev = MKDEV(Major, 0);
    ret = cdev_add(my_cdev, dev, 1); //  one is the number of device minor numbers
    if (ret < 0)
    {
        printk(KERN_INFO "Unable to allocate cdev.\n");
        return ret;
    }

    printk(KERN_INFO "I was assigned major number %d. \n", Major);
    printk(KERN_INFO "To use the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return 0;
}

/*
 * This function is called when the module is unloaded
 */
void 
cleanup_module(void)
{
    cdev_del(my_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device %s, major %d unregistered.\n", DEVICE_NAME, Major);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev_SDP_1"
 */
static int 
device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "open\n");

    if (down_interruptible(&me))
    {
        printk(KERN_INFO "could not hold semaphore\n");
        return -1;
    }

    return 0;
}

/* 
 * Called when a process closes the device file.
 */
static int 
device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "release\n");
    up(&me);
    return 0;
}

/* 
 * Called when a process reads from the device file.
 */
ssize_t 
device_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    unsigned long ret;
    printk(KERN_INFO "read\n");

    /* Limit count to the buffer length to avoid overflows */
    count = (count < strlen(buf_ptr)) ? count : strlen(buf_ptr);
    printk(KERN_INFO "count: %d\n", (int) count);

    /* Copy data to user */
    ret = copy_to_user(buff, buf_ptr, count);

    /* Update the buffer pointer */
    buf_ptr = buf_ptr + count;

    return count;
}

/* 
 * Called when a process writes to the device file.
 */
ssize_t 
device_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
    unsigned long ret;
    printk(KERN_INFO "write\n");

    /* Clean buffer contents */
    memset(char_dev_buf, 0, BUF_LEN);

    /* Limit count to the buffer length to avoid overflows */
    count = (count < BUF_LEN - 1) ? count : BUF_LEN - 1;
    printk(KERN_INFO "count: %d\n", (int) count);

    /* Copy data from user */
    ret = copy_from_user(char_dev_buf, buff, count);

    /* Reset the buffer pointer */
    buf_ptr = char_dev_buf;

    return count;
}