#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>	
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h> 

#define  DEVICE_NAME "kforth"
#define  CLASS_NAME  "k4th"
static int    majorNumber;                  
static struct class*  fClass  = NULL; 
static struct device* fDevice = NULL; 

static char *forth_output;
static int size_of_output=0;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static DEFINE_MUTEX(kforth_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};


static int __init kforth_init(void)
{
   
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Kforth failed to register a major number\n");
      return majorNumber;
   }
   // Register the device class
   fClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(fClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(fClass);          // Correct way to return an error on a pointer
   }
 
   // Register the device driver
   fDevice = device_create(fClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(fDevice)){               // Clean up if there is an error
      class_destroy(fClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(fDevice);
   }
   mutex_init(&kforth_mutex);
   
   return 0;
}

static void __exit kforth_exit(void)
{
	mutex_destroy(&kforth_mutex);
	device_destroy(fClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(fClass);                          // unregister the device class
   class_destroy(fClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
}

static int dev_open(struct inode *inodep, struct file *filep){
   if(!mutex_trylock(&kforth_mutex)){    /// Try to acquire the mutex (i.e., put the lock on/down)
                                          /// returns 1 if successful and 0 if there is contention
      printk(KERN_ALERT "Kforth: Device in use by another process");
      return -EBUSY;
   }
   return 0;
}
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, forth_output, size_of_output);
 
   if (error_count==0){            // if true then have success
      return (size_of_output=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "Kforth: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
//   sprintf(message, "%s(%d letters)", buffer, len);   // appending received string with its length
//   size_of_message = strlen(message);                 // store the length of the stored message

   return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
	mutex_unlock(&kforth_mutex); 
   return 0;
}

module_init(kforth_init);
module_exit(kforth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("y.salnikov");
MODULE_DESCRIPTION("forth interpreter in kernel space");
MODULE_SUPPORTED_DEVICE("forth");
