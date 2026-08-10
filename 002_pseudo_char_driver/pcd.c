#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/version.h>
#include<linux/uaccess.h>

#define DEV_MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* pseudo device memory*/
char device_buffer[DEV_MEM_SIZE];

/* This holds device number*/
dev_t device_number;

/* Cdev Variable*/
struct cdev pcd_cdev;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
	loff_t temp;

	pr_info("lseek requested\n");
	pr_info("Current file pposition is %lld\n", filp->f_pos);

	switch(whence){
		case SEEK_SET:
			if((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;

		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;

		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	
	pr_info("updated file position = %lld\n", filp->f_pos);
	return filp->f_pos;;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	pr_info("read requested for %zu bytes \n", count);
	pr_info("Current file position = %lld\n", *f_pos);

	/* Adjust the count*/
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	/* Copy to user */
	if(copy_to_user(buff, &device_buffer[*f_pos], count)){
	       return -EFAULT;
	}

	/*Update the current file pos*/
	*f_pos += count;

	pr_info("Updated file position = %lld\n", *f_pos);

	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	pr_info("write requested for %zu bytes \n", count);
	pr_info("Current file position before write = %lld\n", *f_pos);

	/* Adjust the count */
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	if(!count)
		return -ENOMEM;

	/* copy from user */
	if(copy_from_user(&device_buffer[*f_pos], buff, count))
		return -EFAULT;

	/*Update the current file position*/
        *f_pos += count;

	pr_info("Number of bytes successfully written = %zu\n", count);
	pr_info("Updated file position = %lld\n", *f_pos);
	
	return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	pr_info("open was successful\n");
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

struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_driver_init(void)
{
	int ret;

	/*1.Dynamically allocating the device number*/
	ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");
	if(ret < 0){
		pr_info("Char dev failed\n");
		goto out;
	}

	pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

	/*2.Initialise the Cdev structure with fops*/
	cdev_init(&pcd_cdev, &pcd_fops);

	/*3.Register a device(cdev structure) with VFS*/
	pcd_cdev.owner = THIS_MODULE;
	ret = cdev_add(&pcd_cdev, device_number, 1);
	if(ret < 0){
		pr_info("Cdev_add failed\n");
		goto unreg_chrdev;
	}

	/*4.Create class under /sys/class/ */

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
		class_pcd = class_create("pcd_class");

	#else
		class_pcd = class_create(THIS_MODULE, "pcd_class");

	#endif
	if(IS_ERR(class_pcd))
	{
		pr_err("Class creation failed\n");
		ret = PTR_ERR(class_pcd);
		goto cdev_del;
	}

	/*5.Populate the sysfs with device information 
	 * class_p, parent_device, device num, device info, device_file_name*/
	device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
	if(IS_ERR(device_pcd)){
		pr_err("Device create failed\n");
		ret = PTR_ERR(device_pcd);
		goto class_del;
	}
	pr_info("Module init was successful\n");

	return 0;

class_del:
	class_destroy(class_pcd);
cdev_del:
	cdev_del(&pcd_cdev);
unreg_chrdev:
	unregister_chrdev_region(device_number, 1);
out:
	pr_err("Module init failed\n");
	return ret;
}

static void __exit driver_cleanup(void)
{
	device_destroy(class_pcd, device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rushikesh");
MODULE_DESCRIPTION("Pseudo driver code\n");
