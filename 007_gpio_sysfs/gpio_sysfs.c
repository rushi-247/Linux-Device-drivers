#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/version.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include<linux/slab.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>
#include<linux/of_device.h>
#include<linux/device.h>
#include<linux/gpio/consumer.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Device Private data structure */
struct gpiodev_private_data
{
	char label[20];
    struct gpio_desc *desc;
};

/*Driver private data structure*/
struct gpiodrv_private_data
{
	int total_devices;
	struct class *class_gpio;
    struct device **dev;
};

struct gpiodrv_private_data gpio_drv_data;

ssize_t label_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    return snprintf(buf, PAGE_SIZE, "%s\n", dev_data->label);
}

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    int value = gpiod_get_value(dev_data->desc);
    return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    int value;

    if (kstrtoint(buf, 10, &value) < 0)
        return -EINVAL;

    gpiod_set_value(dev_data->desc, value);
    return count;
}

ssize_t direction_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    int direction = gpiod_get_direction(dev_data->desc);
    return snprintf(buf, PAGE_SIZE, "%s\n", direction == 0 ? "input" : "output");
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    int ret;

    if(sysfs_streq(buf, "in")) {
        ret = gpiod_direction_input(dev_data->desc);
    } else if(sysfs_streq(buf, "out")) {
        ret = gpiod_direction_output(dev_data->desc, 0);
    } else {
        return -EINVAL;
    }
    return ret ? ret : count;
}

static DEVICE_ATTR_RO(label);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RW(direction);

static struct attribute *gpio_attrs[] = {
    &dev_attr_label.attr,
    &dev_attr_value.attr,
    &dev_attr_direction.attr,
    NULL
};

static struct attribute_group gpio_attr_group = {
    .attrs = gpio_attrs
};

static const struct attribute_group *gpio_attr_groups[] = {
    &gpio_attr_group,
    NULL
};

int gpio_sysfs_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    const char *name;
    int ret;

    /*parent device node*/
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;

    struct gpiodev_private_data *dev_data;

    gpio_drv_data.total_devices = of_get_child_count(parent);
    if(!gpio_drv_data.total_devices) {
        dev_err(dev, "No child nodes found in device tree\n");
        return -ENODEV;
    }

    dev_info(dev, "Total child nodes found: %d\n", gpio_drv_data.total_devices);

    gpio_drv_data.dev = devm_kzalloc(dev, sizeof(struct device *) * gpio_drv_data.total_devices, GFP_KERNEL);    

    int i = 0;

    for_each_available_child_of_node(parent, child)
    {
        dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
        if (!dev_data) {
            dev_err(dev, "Cannot allocate memory for device private data\n");
            return -ENOMEM; 
        }

        if(of_property_read_string(child, "label", &name)) {
            dev_warn(dev, "Missing label information\n");
            snprintf(dev_data->label, sizeof(dev_data->label), "unkngpio-%d", i);
        }
        else{
            strcpy(dev_data->label, name);
            dev_info(dev, "GPIO label: %s\n", dev_data->label);
        }
        dev_data->desc = devm_fwnode_get_gpiod_from_child(dev,"bone", &child->fwnode, GPIOD_ASIS, dev_data->label);
        if (IS_ERR(dev_data->desc)) {
            ret = PTR_ERR(dev_data->desc);
            if(ret == -ENOENT)
                dev_err(dev, "No GPIO has been assigned to the requested function\n");
            return ret;
        }

        /*set the gpio direction to output*/
        ret = gpiod_direction_output(dev_data->desc, 0);
        if (ret) {
            dev_err(dev, "Failed to set GPIO direction\n");
            return ret;
        }

        gpio_drv_data.dev[i] = device_create_with_groups(gpio_drv_data.class_gpio, dev, 0, dev_data, gpio_attr_groups, dev_data->label);
        if(IS_ERR(gpio_drv_data.dev[i])) {
            dev_err(dev, "Failed to create device for GPIO\n");
            return PTR_ERR(gpio_drv_data.dev[i]);
        }
        
        i++;
    }

    return 0;
}

int gpio_sysfs_remove(struct platform_device *pdev)
{
    int i = 0;

    dev_info(&pdev->dev, "Removing GPIO sysfs driver\n");

    for(i = 0; i < gpio_drv_data.total_devices; i++) {
        device_unregister(gpio_drv_data.dev[i]);
    }
    return 0;
}

struct of_device_id gpio_device_match[] = {
    {.compatible = "org,bone-gpio-sysfs"},
    {}
};

struct platform_driver gpiosysfs_platform_driver = 
{
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    .driver = {
                .name = "bone-gpio-sysfs",
                .of_match_table = of_match_ptr(gpio_device_match)
    }
};

int __init gpio_sysfs_init(void)
{

    /*1. Create a class*/
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        gpio_drv_data.class_gpio = class_create("bone_gpio");

	#else
        gpio_drv_data.class_gpio = class_create(THIS_MODULE, "bone_gpio");
    #endif

    if (IS_ERR(gpio_drv_data.class_gpio)) {
        pr_err("Class creation failed\n");
        return PTR_ERR(gpio_drv_data.class_gpio);
    }

    platform_driver_register(&gpiosysfs_platform_driver);
    pr_info("GPIO sysfs driver initialized\n");
    return 0;
}

void __exit gpio_sysfs_exit(void)
{
    platform_driver_unregister(&gpiosysfs_platform_driver);
    class_destroy(gpio_drv_data.class_gpio);
    pr_info("GPIO sysfs driver exited\n");
}

module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rushikesh");
MODULE_DESCRIPTION("Pseudodriver code for n devices\n");