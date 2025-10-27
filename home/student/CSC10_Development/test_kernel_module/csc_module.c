#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of.h>

#define HW_REGS_BASE 0xff200000
#define HW_REGS_SPAN 0x00200000
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define LED_PIO_BASE 0x10
#define SW_BASE 0x00

#define DEVNAME "PIO Interrupt Handler"

// lightweight bridge base address
static void *LW_virtual;
// virtual address of leds PIO
static volatile unsigned int *LEDR_ptr;
// virtual address of switches PIO
static volatile unsigned int *SW_ptr;
// irq number for switches PIO
static int irq_number;

irq_handler_t irq_handler (int irq, void *dev_id)
{
	*(SW_ptr + 3) = 0xF;

	printk(KERN_INFO DEVNAME "IRQ called!\n");
	return IRQ_HANDLED;
}

static int init_handler(struct platform_device *pdev)
{
	// map the lightweight bridge into virtual memory
	LW_virtual = ioremap(HW_REGS_BASE, HW_REGS_SPAN);
	if (LW_virtual == NULL) {
		printk(KERN_ALERT DEVNAME "ERROR: ioremap failed\n");
		return -EINVAL;
	}

	LEDR_ptr = LW_virtual + LED_PIO_BASE; 
	SW_ptr = LW_virtual + SW_BASE;
	// switch all LEDs on
	*LEDR_ptr = 0xFF;

	*(SW_ptr + 3) = 0xF;
	*(SW_ptr + 2) = 0xF;

	irq_number = platform_get_irq(pdev, 0);
	printk(KERN_INFO DEVNAME ": IRQ %d is being registered!\n", irq_number);

	int err = request_irq(irq_number, irq_handler, 0, DEVNAME, NULL);
	if (err != 0) {
		printk(KERN_ALERT DEVNAME "ERROR: IRQ %d can not be registered\n", irq_number);
	}
	return err;
}
static int clean_handler(struct platform_device *pdev)
{
	printk(KERN_INFO DEVNAME ": IRQ %d is being unregistered!\n", irq_number);

	// switch all LEDs off
	*LEDR_ptr = 0;
	// unmap the LW bridge 
	iounmap(LW_virtual);
	// unregister the IRQ
	free_irq(irq_num, NULL);
	return 0;
}

// describe which device we want to bind to this kernel module
// this must match an entry in the device tree
static const struct of_device_id mijn_module_id[] ={
	{.compatible = "altr,switches"},
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
	.remove = clean_handler
};

// register this platform driver
module_platform_driver(mijn_module_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniël Versluis, Harry Broeders");
MODULE_DESCRIPTION("Kernel module to handle PIO interrupt from DE1-SoC board");
