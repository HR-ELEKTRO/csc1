#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>

static void *mem_ptr;
static volatile unsigned char *LED_ptr;

static int __init hello_init(void) {
	mem_ptr = ioremap(0xFF200010, 1);
	if (mem_ptr == NULL) {
		pr_err("ERROR: memory of device can not accessed\n");
		return -ENOMEM;
	}
	LED_ptr = mem_ptr;
	*LED_ptr |= 0x01; 
	return 0;
}

static void __exit hello_exit(void) {
	*LED_ptr &= ~0x01;
	iounmap (mem_ptr);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harry Broeders");
MODULE_DESCRIPTION("Led kernel module");
