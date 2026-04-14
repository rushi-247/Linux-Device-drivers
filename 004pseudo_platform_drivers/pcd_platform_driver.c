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
	pr_info("A device is connected\n");
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

static int __init pcd_platform_driver_init(void)
{
	platform_driver_register(&pcd_platform_driver);
	pr_info("pcd platform driver loaded\n");
	return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
	platform_driver_unregister(&pcd_platform_driver);
	pr_info("pcd platform driver unloaded\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rushikesh");
MODULE_DESCRIPTION("Pseudo driver code for n devices\n");
