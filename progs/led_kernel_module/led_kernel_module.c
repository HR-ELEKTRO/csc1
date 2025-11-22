#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>

static void __iomem *io_ptr;

static int __init led_init(void) {
	io_ptr = ioremap(0xFF200010, 4);
	if (io_ptr == NULL) {
		pr_err("ERROR: memory of device can not accessed\n");
		return -ENOMEM;
	}
	writel(readl(io_ptr) | (1<<0), io_ptr);
	return 0;
}

static void __exit led_exit(void) {
	writel(readl(io_ptr) & ~(1<<0), io_ptr);
	iounmap(io_ptr);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harry Broeders");
MODULE_DESCRIPTION("Led kernel module");
