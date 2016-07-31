#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>	
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/circ_buf.h>
#include "forth.h"

#define  DEVICE_NAME "kforth"
#define  CLASS_NAME  "k4th"
#define  CHUNK_SIZE 512

static int    majorNumber;                  
static struct class*  fClass  = NULL; 
static struct device* fDevice = NULL; 
static forth_context_type *fc;
static char *in_buf, *out_buf;

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
   
   mutex_init(&kforth_mutex);
   in_buf=kmalloc(CHUNK_SIZE,GFP_KERNEL);
   out_buf=kmalloc(CHUNK_SIZE,GFP_KERNEL);
   if(in_buf==NULL) return -ENOMEM;
   if(out_buf==NULL) return -ENOMEM;
   fc=forth_init();

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
  
   return 0;
}

static void __exit kforth_exit(void)
{
	forth_done(fc);
	mutex_destroy(&kforth_mutex);
	device_destroy(fClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(fClass);                          // unregister the device class
	class_destroy(fClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number

   kfree(in_buf);
   kfree(out_buf);
}

static int dev_open(struct inode *inodep, struct file *filep){
   if(!mutex_trylock(&kforth_mutex)){    
      printk(KERN_ALERT "Kforth: Device in use by another process");
      return -EBUSY;
   }
   return 0;
}
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{

	size_t err;
	size_t i;
	size_t to_read;
	size_t lngth;


	lngth=CIRC_CNT(fc->out.head,fc->out.tail,TIB_SIZE);
	if(lngth==0) return 0;
	to_read=len;
	
	if (len>CHUNK_SIZE) to_read=CHUNK_SIZE;
	if (to_read>lngth)  to_read=lngth;


	for(i=0;i<to_read;i++)
	{
		out_buf[i]=read_from_out(fc);
	}
	if(to_read>0)
	{
		err=copy_to_user(buffer,out_buf,to_read);
		if(err) printk(KERN_ALERT "Kforth: Can't write to userspace\n");
	}
	return to_read;
	return 0;
}
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	size_t transfered=0;
	size_t chunk_size;
	size_t err;
	size_t i;
	
	while(transfered<len)
	{
		if((len-transfered)<512) chunk_size=len-transfered;
		else chunk_size=CHUNK_SIZE;
		err=copy_from_user(in_buf,buffer+transfered,chunk_size);
		if(err) printk(KERN_ALERT "Kforth: Can't read from userspace\n");
		for(i=0;i<chunk_size;i++)
		{
			put_to_in(fc,in_buf[i]);
		}
		transfered+=chunk_size;
	}
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
