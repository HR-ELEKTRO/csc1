#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of.h>

MODULE_LICENSE("GPL");

#define HW_REGS_BASE 0xff200000
#define HW_REGS_SPAN 0x00200000
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define LED_PIO_BASE 0x10
#define SW_BASE 0x00

#define DEVNAME "Mijn Module"

void * LW_virtual; // Lightweight bridge base address
volatile int *LEDR_ptr; // virtual addresses
volatile int *SW_ptr;

irq_handler_t irq_handler (int irq, void *dev_id, struct pt_regs * regs)
{
	*(SW_ptr + 3) = 0xF;

	printk(KERN_ALERT "In de IRQ!");
	return (irq_handler_t) IRQ_HANDLED;
}

static int init_handler(struct platform_device * pdev)
{
	int irq_num,ret;

	//Koppel fysiek geheugenbereik aan pointer
	LW_virtual = ioremap(HW_REGS_BASE, HW_REGS_SPAN);

	LEDR_ptr = LW_virtual + LED_PIO_BASE; //offset naar PIO registers
	SW_ptr = LW_virtual + SW_BASE;
	*LEDR_ptr = 0xFF; //Alle leds aanzetten

	*(SW_ptr + 3) = 0xF;
	*(SW_ptr + 2) = 0xF;

	irq_num = platform_get_irq(pdev,0);
	printk(KERN_ALERT DEVNAME ": IRQ %d wordt geregistreert!\n", irq_num);

	ret = request_irq(irq_num, (irq_handler_t) irq_handler, 0, DEVNAME, NULL);

	return ret;
}
static int  clean_handler(struct platform_device *pdev)
{
	int irq_num;
	irq_num=platform_get_irq(pdev,0);
	printk(KERN_ALERT DEVNAME ": IRQ %d wordt vrijgegeven!\n", irq_num);

	*LEDR_ptr = 0; // Alle leds uitzettten
	iounmap (LW_virtual); //mapping ongedaan maken
	free_irq(irq_num, NULL);
	return 0;
}


/*
* Hierin beschrijven we welk device we willen koppelen
* aan onze module. Deze moet in de device-tree overeen-
* komen! 
*/
static const struct of_device_id mijn_module_id[] ={
	{.compatible = "altr,switches"},
	{}
};

//handlers e.d. koppelen
static struct platform_driver mijn_module_driver = {
	.driver = {
	 	.name = DEVNAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mijn_module_id),
	},
	.probe = init_handler,
	.remove = clean_handler
};

//module registreren
module_platform_driver(mijn_module_driver);
