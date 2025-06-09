#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/err.h>
#include <linux/delay.h>

#define gpio17 17

#define DEVICE_NAME "gpio_char_device"
#define CLASS_NAME "gpio_char_class"
static struct class *gpio_char_class;
static struct cdev gpio_char_dev;
dev_t dev = 0;

static int gpio_open(struct inode *inode, struct file *file);
static int gpio_release(struct inode *inode, struct file *file);
static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
staitc ssize_t gpio_write(struct file *filp, const char *buf, size_t len, loff_t *off);

static struct file_operations fops={
    .owner = THIS_MODULE,
    .open = gpio_open,
    .release = gpio_release,
    .read = gpio_read,
    .write = gpio_write,
}

static int gpio_open(struct inode *inode, struct file *file)
{
    pr_info("Gpio open...\n");
    return 0;
}

static int gpio_release(struct inode *inode, struct file *file)
{
    pr_info("Gpio closed...\n");
    return 0;
}

static ssize_t gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    uint8_t gpio_state = 0;

    // reading gpio value
    gpio_state = gpio_get_value(gpio17);

    //write to user
    len = 1;
    if(copy_to_user(buf, &gpio_state, len) > 0){
        pr_err("Error: Failed to copy data to user space\n");
    }

    pr_info("Read function: GPIO17 = %d \n", gpio_state);

    return 0;
}

static ssize_t gpio_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    uint8_t rec_buf[10] = {0};
    if(copy_from_user(rec_buf, buf, len) > 0){
        pr_err("Error: Failed to copy data from user space\n");
    }

    pr_info("Write function: GPIO17 set = %c\n", rec_buf[0]);

    if(rec_buf[0] == '1'){
        gpio_set_value(gpio17, 1);
        pr_info("GPIO17 set to HIGH\n");
    } else if(rec_buf[0] == '0') {
        gpio_set_value(gpio17, 0);
        pr_info("GPIO17 set to LOW\n");
    } else {
        pr_err("Unknow command: Please provide either 1 or 0 \n");
    }

    return len;
}

static int __init gpio_init(void)
{
    if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0){
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&gpio_char_dev, &fops);

    if(cdev_add(&gpio_char_dev, 1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }

    if(IS_ERR(dev_class = class_create(CLASS_NAME))){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    if(IS_ERR(device_create(gpio_char_class, NULL, dev, NULL, DEVICE_NAME))){
        pr_err("Cannot create the device\n");
        goto r_device;
    }

    // Requesting the GPIO
    if(gpio_request(gpio17, "GPIO17") < 0){
        pr_err("Error: GPIO %d request\n", gpio17);
        goto r_gpio;
    }

    // configure the gpio as output
    gpio_direction_output(gpio17, 0);

    gpio_export(gpio17, false);;

    pr_info("GPIO character device driver initialized\n");
    return 0;

r_gpio:
    gpio_free(gpio17);
r_device:
    device_destroy(gpio_char_class, dev);
r_class:
    class_destroy(gpio_char_class);
r_del:
    cdev_del(&gpio_char_dev);
r_unreg:
    unregister_chrdev_region(dev,1);

    return -1;
}


static void __exit gpio_exit(void)
{
    gpio_unexport(gpio17);
    gpio_free(gpio17);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&gpio_char_dev);
    unregister_chrdev_region(dev, 1);
    pr_info("GPIO character device driver removed\n");

}

module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TRANH LINUX");
MODULE_DESCRIPTION("A simple GPIO character device driver");
MODULE_VERSION("1.0");