#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define DEVNAME "MYLEDDEV"

// data can be global because there will be no minor devices

// virtual address of leds PIO
static volatile unsigned int *PIO_ptr;

// device number
static dev_t dev = 0;
// device major number
static int dev_major = 0;

// device class structure
static struct class *my_dev_class = NULL;

// char device structure
static struct cdev my_dev_cdev;

static int my_dev_open(struct inode *inode, struct file *file) 
{
	printk(KERN_INFO DEVNAME ": Device open\n");
	if ((file->f_flags & O_WRONLY) != O_WRONLY) {
		printk(KERN_ALERT DEVNAME ": ERROR: Device opened not in write only mode\n");
		return -EACCES;
	} 
	return 0;
}

static int my_dev_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO DEVNAME ": Device close\n");
	return 0;
}

static ssize_t my_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	char buffer[128];
	size_t maxdatalen = sizeof buffer;
	printk(KERN_INFO DEVNAME ": Device write\n");

	if (count < maxdatalen) {
		maxdatalen = count;
	}

	unsigned long ret = copy_from_user(buffer, buf, maxdatalen);
	if (ret) {
		printk(KERN_ALERT DEVNAME ": ERROR: copy_from_user failed\n");
		return ret;
	}

	size_t i;
	for (i = 0; i < maxdatalen; i++) {
		int value = (int)buffer[i];
		printk(KERN_INFO DEVNAME ": Data written 0x%02X\n", value);
		*PIO_ptr = value;
	}

	return maxdatalen;
}

// initialize file_operations
static const struct file_operations my_dev_fops = {
	.open = my_dev_open,
	.release = my_dev_release,
	.write = my_dev_write
};

// callback function called when device is added to class
// used to set the permissions of the device node to 0666 (read and write for all users)
static int my_dev_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
	int ret = add_uevent_var(env, "DEVMODE=%#o", 0666);
	if (ret < 0) {
		printk(KERN_ALERT DEVNAME ": ERROR: add_uevent_var failed\n");
		return ret;
	}
	return 0;
}

static int init_handler(struct platform_device *pdev)
{
	printk(KERN_INFO DEVNAME ": Create character device\n");

	// allocate a range of char device numbers
	// in this case only one minor number
	// the major number will be assigned dynamically
	int err = alloc_chrdev_region(&dev, 0, 1, "leds");
	if (err < 0) {
		printk(KERN_ALERT DEVNAME ": ERROR: no major device number available\n");
		return err;
	}
	// extract the assigned major device number
	dev_major = MAJOR(dev);

	// create device class
	// class leds is a reserved class name and can not be used. Therefore we use my_leds
	my_dev_class = class_create("my_leds");

	if (IS_ERR(my_dev_class)) {
		printk(KERN_ALERT DEVNAME ": ERROR: class can not be created for device\n");
		err = PTR_ERR(my_dev_class);
		goto cleanup_chrdev_region;
	}
	// my_dev_uevent is called when a device is added to this class
	my_dev_class->dev_uevent = my_dev_uevent;

	// init new device
	cdev_init(&my_dev_cdev, &my_dev_fops);
	my_dev_cdev.owner = THIS_MODULE;

	// create device in /sys/devices/virtual/my_leds/leds
	err = cdev_add(&my_dev_cdev, MKDEV(dev_major, 0), 1);
	if (err < 0) {
		printk(KERN_ALERT DEVNAME ": ERROR: device number can not be added for device\n");
		goto cleanup_class;
	}

	// create device node /dev/leds
	struct device *my_dev = device_create(my_dev_class, NULL, MKDEV(dev_major, 0), NULL, "leds");
	if (IS_ERR(my_dev)) {
		printk(KERN_ALERT DEVNAME ": ERROR: device can not be created for driver\n");
		err = PTR_ERR(my_dev);
		goto cleanup_cdev;
	}

	// map the PIO device registers into virtual memory
	void *mem_ptr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(mem_ptr)) {
		printk(KERN_ALERT DEVNAME ": ERROR: no base address found for PIO device\n");
		err = PTR_ERR(mem_ptr);
		goto cleanup_device;
	}
	PIO_ptr = mem_ptr;
	return 0;
	// cleanup on errors
cleanup_device:	
	device_destroy(my_dev_class, MKDEV(dev_major, 0));
cleanup_cdev:
	cdev_del(&my_dev_cdev);
cleanup_class:
	class_destroy(my_dev_class);
cleanup_chrdev_region:
	unregister_chrdev_region(dev, 1);
	return err;
}

static void clean_handler(struct platform_device *pdev)
{
	printk(KERN_INFO DEVNAME ": Destroy character device\n");

	device_destroy(my_dev_class, MKDEV(dev_major, 0));
	cdev_del(&my_dev_cdev);
	class_destroy(my_dev_class);
	unregister_chrdev_region(dev, 1);
}

// describe which device we want to bind to this kernel module
// this must match an entry in the device tree
static const struct of_device_id mijn_module_id[] ={
	{.compatible = "leds"},
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
MODULE_DESCRIPTION("Character device driver for LEDR7 downto LEDR0 on DE1-SoC board");