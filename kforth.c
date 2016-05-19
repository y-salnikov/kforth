#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>	
#include <linux/tty_driver.h>

static int __init kforth_init(void)
{
	/* allocate the tty driver */
	forth_tty_driver = alloc_tty_driver(TINY_TTY_MINORS);
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
