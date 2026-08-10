#include "pcd_platform_driver_dt_sysfs.h"

struct pcdrv_private_data pcdrv_data;

struct device_config pcdev_config[] = 
{
	[PCDEVA1X] = {.config_item1 = 60, .config_item2 = 21},
	[PCDEVB1X] = {.config_item1 = 50, .config_item2 = 22},
	[PCDEVC1X] = {.config_item1 = 40, .config_item2 = 23},
	[PCDEVD1X] = {.config_item1 = 30, .config_item2 = 24}
};

struct platform_device_id pcdevs_ids[] =
{
	{.name = "pcdev-A1X", .driver_data = PCDEVA1X},
	{.name = "pcdev-B1X", .driver_data = PCDEVB1X},
	{.name = "pcdev-C1X", .driver_data = PCDEVC1X},
	{.name = "pcdev-D1X", .driver_data = PCDEVD1X},
	{}
};

struct of_device_id org_pcdev_dt_match[] = 
{
	{.compatible = "pcdev-A1X", .data = (void *)PCDEVA1X},
	{.compatible = "pcdev-B1X", .data = (void *)PCDEVB1X},
	{.compatible = "pcdev-C1X", .data = (void *)PCDEVC1X},
	{.compatible = "pcdev-D1X", .data = (void *)PCDEVD1X},
	{}
};

/*file operations of the driver*/
struct file_operations pcd_fops = {
	.owner = THIS_MODULE,
	.read  = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.open  = pcd_open,
	.release = pcd_release
};

ssize_t show_max_size(struct device *dev, struct device_attribute *attr, char *buf)
{
	/*get access to device private data*/
	struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
	return sprintf(buf, "%d\n", dev_data->pdata.size);
}

ssize_t store_max_size(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	long result;
	int ret;
	struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

	ret = kstrtol(buf, 10, &result);
	if(ret < 0)
		return ret;

	dev_data->pdata.size = result;

	dev_data->buffer = devm_krealloc(dev, dev_data->buffer, dev_data->pdata.size, GFP_KERNEL);

	return count;
}

ssize_t show_serial_number(struct device *dev, struct device_attribute *attr, char *buf)
{
	/*get access to device private data*/
	struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
	return sprintf(buf, "%s\n", dev_data->pdata.serial_number);
}

//creating 2 variables of struct device attribute for sysfs files
static DEVICE_ATTR(max_size,S_IRUGO | S_IWUSR, show_max_size, store_max_size);
static DEVICE_ATTR(serial_number,S_IRUGO, show_serial_number, NULL);

struct attribute *pcd_attrs[] = {
	&dev_attr_max_size.attr,
	&dev_attr_serial_number.attr,
	NULL
};

struct attribute_group pcd_attr_group = {
	.attrs = pcd_attrs
};

int pcd_sysfs_create_files(struct device *pcd_dev){
	int ret;
	return sysfs_create_group(&pcd_dev->kobj, &pcd_attr_group);
	#if 0
	ret = sysfs_create_file(&pcd_dev->kobj, &dev_attr_max_size.attr);
	if(ret)
		return ret;

	return sysfs_create_file(&pcd_dev->kobj, &dev_attr_serial_number.attr);
	#endif
}

//gets called when device is removed from the system
void pcd_platform_driver_remove(struct platform_device *pdev)
{
	struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);

	/*1. Remove a device that was created with device_create() */
	device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);

	/*2. Remove a cdev entry from the system*/
	cdev_del(&dev_data->cdev);

	pcdrv_data.total_devices--;

	dev_info(&pdev->dev, "A device is removed\n");
	//return 0;
}

//called when device tree is used to get platform data
struct pcdev_platform_data *pcdev_get_platdata_from_dt(struct device *dev)
{
	struct device_node *dev_node = dev->of_node;
	struct pcdev_platform_data *pdata;

	if(!dev_node){
		dev_info(dev, "Device tree node is not available\n");
		return ERR_PTR(-EINVAL);
	}

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if(!pdata){
		pr_info("Cannot allocate memory\n");
		return ERR_PTR(-ENOMEM);
	}

	if(of_property_read_string(dev_node, "org,device-serial_num", &pdata->serial_number)){
		pr_info("Missing serial number property\n");
		return ERR_PTR(-EINVAL);
	}

	if(of_property_read_u32(dev_node, "org,size", &pdata->size)){
		pr_info("Missing size property\n");
		return ERR_PTR(-EINVAL);
	}

	if(of_property_read_u32(dev_node, "org,perm", &pdata->perm)){
		pr_info("Missing permission property\n");
		return ERR_PTR(-EINVAL);
	}

	return pdata;
}

//gets called when matched device is found
int pcd_platform_driver_probe(struct platform_device *pdev)
{

	int ret;
	
	struct pcdev_private_data *dev_data;

	struct pcdev_platform_data *pdata;

	struct device *dev = &pdev->dev;

	int driver_data;

	/*used to store the matched device tree entry*/
	const struct of_device_id *match;

	dev_info(dev, "A Device is detected\n");
	
	/* match will always be NULL if Linux doesn't support device tree i.e CONFIG_OF is off */
	match = of_match_device(of_match_ptr(org_pcdev_dt_match),dev);

	if(match)
	{
		pdata = pcdev_get_platdata_from_dt(dev);

		if(IS_ERR(pdata)){
			return PTR_ERR(pdata);
		}
		driver_data = (int)match->data;
	}
	else{
		pdata = (struct pcdev_platform_data *) dev_get_platdata(dev);
		driver_data = pdev->id_entry->driver_data;
	}

	if(!pdata)
	{
		dev_info(dev, "No platform data available\n");
		return -EINVAL;
	}

	/*2.Dynamically allocate memory for the device private bus*/
	dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
	if(!dev_data){
		dev_info(dev, "Cannot allocate memory\n");
		return -ENOMEM;
	}

	/*save the device private data pointer in platform device structure*/
	dev_set_drvdata(&pdev->dev, dev_data);
	
	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
	dev_data->pdata.serial_number = pdata->serial_number;

	dev_info(dev, "Device serial number = %s\n", dev_data->pdata.serial_number);
	dev_info(dev, "Device Size = %d\n", dev_data->pdata.size);
	dev_info(dev, "Device permission = %d\n", dev_data->pdata.perm);

	dev_info(dev, "Config item1 = %d\n", pcdev_config[driver_data].config_item1 );
	dev_info(dev, "Config item2 = %d\n", pcdev_config[driver_data].config_item2 );

	/*3.Dynamically allocate memory for the device buffer using size information
	from the platform data */
	dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
	if(!dev_data->buffer){
		dev_info(dev, "Cannot allocate memory\n");
		return -ENOMEM;
	}
	
	/*4.Get the device number */
	dev_data->dev_num = pcdrv_data.device_num_base + pcdrv_data.total_devices;
	
	/*5.Do cdev init and cdev add*/
	cdev_init(&dev_data->cdev, &pcd_fops);

	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
	if(ret < 0){
		dev_err(dev, "Cdev add failed\n");
		return ret;
	}
	
	/*6.Create device file for the detected platform device*/
	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, &pdev->dev, dev_data->dev_num, NULL, "pcdev-%d", pcdrv_data.total_devices);
	if(IS_ERR(pcdrv_data.device_pcd)){
		dev_err(dev, "Device create failed\n");
		ret = PTR_ERR(pcdrv_data.device_pcd);
		cdev_del(&dev_data->cdev);
		return ret;
	}

	pcdrv_data.total_devices++;

	ret = pcd_sysfs_create_files(pcdrv_data.device_pcd);
	if(ret){
		device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
		return ret;
	}

	dev_info(dev, "A probe was succesful\n");
	
	return 0;

}

struct platform_driver pcd_platform_driver = 
{
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.id_table = pcdevs_ids,
	.driver = {
				.name = "pseudo-char-device",
				.of_match_table = org_pcdev_dt_match
	}
};

static int __init pcd_platform_driver_init(void)
{
	int ret;

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
