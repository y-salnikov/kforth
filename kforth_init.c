#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>	
#include <linux/tty_driver.h>

static struct tty_driver *forth_tty_driver;


static int __init kforth_init(void)
{

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
