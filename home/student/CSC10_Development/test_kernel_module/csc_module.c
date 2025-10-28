#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define DEVNAME "PIO Interrupt Handler"

// pointer to PIO device registers
static volatile unsigned int *PIO_ptr;
// irq number for switches PIO
static int irq_number;

irqreturn_t irq_handler(int irq, void *dev_id)
{
	*(PIO_ptr + 3) = 0xF;

	static int count = 0;
	count++;

	printk(KERN_INFO DEVNAME ": IRQ called %d time(s)!\n", count);
	return IRQ_HANDLED;
}

static int init_handler(struct platform_device *pdev)
{
	// map the PIO device registers into virtual memory
	void *mem_ptr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(mem_ptr)) {
		printk(KERN_ALERT DEVNAME ": ERROR: no base address found for PIO device\n");
		return PTR_ERR(mem_ptr);
	}
	PIO_ptr = mem_ptr;
	// get the irq number of the PIO device
	irq_number = platform_get_irq(pdev, 0);
	if (irq_number < 0) {
		printk(KERN_ALERT DEVNAME ": ERROR: No IRQ number found for PIO device\n");
		return irq_number;
	}
	printk(KERN_INFO DEVNAME ": IRQ %d is being registered!\n", irq_number);
	// register irq handler
	int err = request_irq(irq_number, irq_handler, 0, DEVNAME, NULL);
	if (err != 0) {
		printk(KERN_ALERT DEVNAME ": ERROR: IRQ %d can not be registered\n", irq_number);
	}

	*(PIO_ptr + 3) = 0x0F;
	*(PIO_ptr + 2) = 0x0F;

	return err;
}
static void clean_handler(struct platform_device *pdev)
{
	printk(KERN_INFO DEVNAME ": IRQ %d is being unregistered!\n", irq_number);
	// unregister the IRQ
	if (free_irq(irq_number, NULL) == NULL) {
		printk(KERN_ALERT DEVNAME ": ERROR: IRQ %d can not be unregistered\n", irq_number);
	}
}

// describe which device we want to bind to this kernel module
// this must match an entry in the device tree
static const struct of_device_id mijn_module_id[] ={
	{.compatible = "switches"},
	{}
};

// platform driver structure linking handlers to events
static struct platform_driver mijn_module_driver = {
	.driver = {
	 	.name = DEVNAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mijn_module_id),
	},
	.probe = init_handler,
	// See https://elixir.bootlin.com/linux/v6.6.22/source/include/linux/platform_device.h#L236 for an explanation why we use .remove_new in stead of .remove
	.remove_new = clean_handler
};

// register this platform driver
module_platform_driver(mijn_module_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniël Versluis, Harry Broeders");
MODULE_DESCRIPTION("Kernel module to handle PIO interrupt from DE1-SoC board");
