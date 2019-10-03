#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>

#define LED_MAJOR 62
#define LED_MINOR 0
#define LED_MINOR_AMOUNT 1
#define LED_GPIO 26

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rene Street");
MODULE_DESCRIPTION("LED device driver for fHAT");

static int devno;

static struct cdev led_cdev;
struct file_operations led_fops;

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

    //statisk allokering af major/minor number
    //create nodes med mknod
    devno = MKDEV(LED_MAJOR, LED_MINOR);

    //Register device
    err = register_chrdev_region(devno, LED_MINOR_AMOUNT, "LED0");
    if(err < 0){
        goto gpio_err;
    }

    //Tell kernel about the cdev structure - Final step
    cdev_init(&led_cdev, &led_fops);
    err = cdev_add(&led_cdev, devno, 1);
    if(err < 0){
        goto err_dev_unregister;
    }

    return 0; //Success

    err_dev_unregister:
        unregister_chrdev_region(devno, LED_MINOR_AMOUNT); //unregister devices if error
    gpio_err:
        gpio_free(LED_GPIO); //release claimed GPIO
    err_exit:
        return err;
}

static void __exit led_exit(void){
    cdev_del(&led_cdev); //Delete added cdev

    unregister_chrdev_region(devno, LED_MINOR_AMOUNT); //unregister device

    gpio_free(LED_GPIO); //release claimed GPIO
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
        printk("Error copying to user");
    }

    *f_pos += valbuf_len;
    return valbuf_len;
}

ssize_t ledgpio_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *f_pos){

    char write_buf[count];
    int write_val;

    unsigned long err = copy_from_user(write_buf, ubuf, count);
    if(err < 0){
        printk("Error copying from user");
    }

    sscanf(write_buf, "%d", &write_val);

    gpio_set_value(LED_GPIO, write_val);

    return count;
}

//Implemented file operations methods
struct file_operations led_fops = {
    .owner      = THIS_MODULE,
    .open       = ledgpio_open,
    .release    = ledgpio_release,
    .read       = ledgpio_read,
    .write      = ledgpio_write
};

module_init(led_init);
module_exit(led_exit);
