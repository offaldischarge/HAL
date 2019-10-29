#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>

#define LED_MAJOR 62
#define LED_MINOR 0
#define LED_MINOR_AMOUNT 1
#define LED_GPIO 26

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rene Street");
MODULE_DESCRIPTION("LED device driver for fHAT");

static dev_t devno;

static struct cdev led_cdev;
struct file_operations led_fops;

static struct platform_driver led_platform_driver;
static struct class *led_class;
static struct device *led_device_101;

static int __init led_init(void){
    //variable til fejl behandling
    int err = 0;

    //request GPIO 16. with NULL label
    err = gpio_request(LED_GPIO, NULL);
    if(err < 0 ){
        goto err_exit;
    }

    //Set GPIO port as output
    gpio_direction_output(LED_GPIO, 0);

    //Allocate device
    err = alloc_chrdev_region(&devno, 0, LED_MINOR_AMOUNT, "led_driver");
    if(err){
        printk("Failed to register device\n");
        goto gpio_err;
    }
    printk("LED driver assigned Major no. %i\n", MAJOR(devno));

    //Tell kernel about the cdev structure - Final step
    cdev_init(&led_cdev, &led_fops);
    err = cdev_add(&led_cdev, devno, LED_MINOR_AMOUNT);
    if(err < 0){
        goto err_dev_unregister;
    }

    led_class = class_create(THIS_MODULE, "led_class"); //Create led class
    if(IS_ERR(led_class)){
        printk("Failed to create led class\n");
        goto err_dev_unregister;
    }

    err = platform_driver_register(&led_platform_driver);
    if(err){
        printk("Error registering device\n");
        goto err_cleanup_class;
    }

    return 0; //Success

    err_cleanup_class:
        class_destroy(led_class);
    err_dev_unregister:
        unregister_chrdev_region(devno, LED_MINOR_AMOUNT); //unregister devices if error
    gpio_err:
        gpio_free(LED_GPIO); //release claimed GPIO
    err_exit:
        return err;
}

static void __exit led_exit(void){
    platform_driver_unregister(&led_platform_driver); //Unregister platform driver

    class_destroy(led_class); //Destroy LED class

    gpio_free(LED_GPIO); //release claimed GPIO

    cdev_del(&led_cdev); //Delete added cdev

    unregister_chrdev_region(devno, LED_MINOR_AMOUNT); //unregister device
}

int ledgpio_open(struct inode *inode, struct file *filep){
    int major, minor;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Opening LED GPIO Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

int ledgpio_release(struct inode *inode, struct file *filep){
    int minor, major;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Closing/Releasing LED GPIO Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

ssize_t ledgpio_read(struct file *filep, char __user *buf,
    size_t count, loff_t *f_pos){

    int val;
    char valbuf[16];

    val = gpio_get_value(LED_GPIO);
    sprintf(valbuf, "%d", val);

    int valbuf_len = strlen(valbuf) + 1;
    valbuf_len = valbuf_len > count ? count : valbuf_len;

    unsigned long err = copy_to_user(buf, valbuf, valbuf_len);
    if(err < 0){
        printk("Error copying to user\n");
    }

    *f_pos += valbuf_len;
    return valbuf_len;
}

ssize_t ledgpio_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *f_pos){

    char write_buf[count];
    int write_val;

    unsigned long err = copy_from_user(write_buf, ubuf, count);
    if(err < 0){
        printk("Error copying from user\n");
    }

    sscanf(write_buf, "%d", &write_val);

    gpio_set_value(LED_GPIO, write_val);

    return count;
}

static int led_pdrv_probe(struct platform_device *pdev){

    led_device_101 = device_create(led_class, NULL, MKDEV(MAJOR(devno), LED_MINOR), NULL, "my_led%d", 101);

    printk("New LED platform device: %s\n", pdev->name);
    return 0;
}

static int led_pdrv_remove(struct platform_device *pdev){

    device_destroy(led_class, MKDEV(MAJOR(devno), LED_MINOR));

    printk("Removing device %s\n", pdev->name);
    return 0;
}

//Implemented file operations methods
struct file_operations led_fops = {
    .owner      = THIS_MODULE,
    .open       = ledgpio_open,
    .release    = ledgpio_release,
    .read       = ledgpio_read,
    .write      = ledgpio_write
};

static const struct of_device_id of_led_platform_device_match[] = {
    { .compatible = "ase, plat_drv",}, {},
};

static struct platform_driver led_platform_driver = {
    .probe  = led_pdrv_probe,
    .remove = led_pdrv_remove,
    .driver = {
        .name = "plat_drv",
        .of_match_table = of_led_platform_device_match,
        .owner = THIS_MODULE,
    },
};

module_init(led_init);
module_exit(led_exit);
