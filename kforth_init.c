#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>	
#include <linux/tty.h>
#include <linux/tty_driver.h>

static struct tty_driver *forth_tty_driver;


static int __init kforth_init(void)
{
	int retval;
static struct tty_operations serial_ops= {
    .open = NULL,
    .close = NULL,
    .write = NULL,
    .write_room = NULL,
    .set_termios = NULL,
}; 
	

	/* allocate the tty driver */
	forth_tty_driver = tty_alloc_driver(1,TTY_DRIVER_REAL_RAW | TTY_DRIVER_UNNUMBERED_NODE);
	if (!forth_tty_driver) return -ENOMEM;
	
	/* initialize the tty driver */
	forth_tty_driver->owner = THIS_MODULE;
	forth_tty_driver->driver_name = "forth_tty";
	forth_tty_driver->name = "ttyFORTH";
	forth_tty_driver->name_base= 0;
	forth_tty_driver->major = 4,
	forth_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
	forth_tty_driver->subtype = SERIAL_TYPE_NORMAL,
	forth_tty_driver->flags = TTY_DRIVER_REAL_RAW |  TTY_DRIVER_UNNUMBERED_NODE,
	forth_tty_driver->init_termios = tty_std_termios;
	forth_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_set_operations(forth_tty_driver, &serial_ops);
	/* register the tty driver */
	retval = tty_register_driver(forth_tty_driver);
	if (retval) {
		printk(KERN_ERR "failed to register forth tty driver");
		put_tty_driver(forth_tty_driver);
		return retval;
	}

	return 0;
}

static void __exit kforth_exit(void)
{
	
}

module_init(kforth_init);
module_exit(kforth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("y.salnikov");
MODULE_DESCRIPTION("forth interpreter in kernel space");
MODULE_SUPPORTED_DEVICE("forth");
