#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/version.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Device Private data structure */
struct pcdev_private_data
{
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/*Driver private data structure*/
struct pcdrv_private_data
{
	int total_devices;
	
	/* This holds device number*/
	dev_t device_num_base;
	
	struct class *class_pcd;
	struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
	return 0;	
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	return 0;
}

int check_permission(int dev_perm, int acc_mode)
{	
	if(dev_perm == RDWR)
		return 0;
	
	//ensures readonly access
	if((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)))
		return 0;

	//ensures writeonly access
	if((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)))
		return 0;

	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("release was successful\n");
	return 0;
}


/*file operations of the driver*/
struct file_operations pcd_fops = {
	.owner = THIS_MODULE,
	.read  = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.open  = pcd_open,
	.release = pcd_release
};

//gets called when device is removed from the system
void pcd_platform_driver_remove(struct platform_device *pdev)
{
	pr_info("A device is removed\n");
	//return 0;
}

//gets called when matched device is found
int pcd_platform_driver_probe(struct platform_device *pdev)
{
	pr_info("A device is detected\n");
	return 0;
}

struct platform_driver pcd_platform_driver = 
{
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
				.name = "pseudo-char-device"
	}
};

#define MAX_DEVICES 10 

static int __init pcd_platform_driver_init(void)
{
	int ret;
	int i;

	/*1.Dynamically allocate a device num for max_devices*/
	ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcd_devices");
	if(ret < 0){
		pr_info("Char dev failed\n");
	}

	/*2.Create device class under /sys/class/ */
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
		pcdrv_data.class_pcd = class_create("pcd_class");

	#else
		pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");

	#endif
	if(IS_ERR(pcdrv_data.class_pcd))
	{
		pr_err("Class creation failed\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
		return ret;
	}

	/*3. Registering platform driver */
	platform_driver_register(&pcd_platform_driver);
	pr_info("pcd platform driver loaded\n");
	return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
	/*1.Unregister the platform driver */
	platform_driver_unregister(&pcd_platform_driver);

	/*2.Class destroy*/
	class_destroy(pcdrv_data.class_pcd);

	/*3.Unregister device numbers for MAX_DEVICES*/
	unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
	
	pr_info("pcd platform driver unloaded\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rushikesh");
MODULE_DESCRIPTION("Pseudo driver code for n devices\n");
