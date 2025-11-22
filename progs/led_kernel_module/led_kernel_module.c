#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>

static void *mem_ptr;
static volatile unsigned char *led_ptr;

static int __init led_init(void) {
	mem_ptr = ioremap(0xFF200010, 1);
	if (mem_ptr == NULL) {
		pr_err("ERROR: memory of device can not accessed\n");
		return -ENOMEM;
	}
	led_ptr = mem_ptr;
	*led_ptr |= 0x01; 
	return 0;
}

static void __exit led_exit(void) {
	*led_ptr &= ~0x01;
	iounmap (mem_ptr);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harry Broeders");
MODULE_DESCRIPTION("Led kernel module");
