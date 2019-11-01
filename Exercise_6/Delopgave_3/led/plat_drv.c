#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/of_gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rene Street");
MODULE_DESCRIPTION("GPIO device driver for fHAT");

static dev_t devno;

static struct cdev gpio_cdev;
struct file_operations gpio_fops;

static struct platform_driver gpio_platform_driver;
static struct class *gpio_class;
static struct device *gpio_device_101;

struct gpio_dev{
    int no; // GPIO number
    int dir; // 0: in, 1: out
};

static struct gpio_dev gpio_devs[255];
static int gpios_len = 255;

static int __init gpio_init(void){
    //variable til fejl behandling
    int err = 0;

    //Allocate device
    err = alloc_chrdev_region(&devno, 0, gpios_len, "gpio_driver");
    if(err){
        printk("Failed to register device\n");
        goto err_exit;
    }
    printk("GPIO driver assigned Major no. %i\n", MAJOR(devno));

    //Tell kernel about the cdev structure - Final step
    cdev_init(&gpio_cdev, &gpio_fops);
    err = cdev_add(&gpio_cdev, devno, gpios_len);
    if(err < 0){
        goto err_dev_unregister;
    }

    gpio_class = class_create(THIS_MODULE, "gpio_class"); //Create gpio class
    if(IS_ERR(gpio_class)){
        printk("Failed to create gpio class\n");
        goto err_dev_unregister;
    }

    err = platform_driver_register(&gpio_platform_driver);
    if(err){
        printk("Error registering device\n");
        goto err_cleanup_class;
    }

    return 0; //Success

    err_cleanup_class:
        class_destroy(gpio_class);
    err_dev_unregister:
        unregister_chrdev_region(devno, gpios_len); //unregister devices if error
    err_exit:
        return err;
}

static void __exit gpio_exit(void){
    platform_driver_unregister(&gpio_platform_driver); //Unregister platform driver

    class_destroy(gpio_class); //Destroy gpio class

    cdev_del(&gpio_cdev); //Delete added cdev

    unregister_chrdev_region(devno, gpios_len); //unregister device
}

int gpio_open(struct inode *inode, struct file *filep){
    int major, minor;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Opening GPIO Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

int gpio_release(struct inode *inode, struct file *filep){
    int minor, major;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Closing/Releasing GPIO Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

ssize_t gpio_read(struct file *filep, char __user *buf,
    size_t count, loff_t *f_pos){

    int val;
    char valbuf[16];

    int minor = iminor(filep->f_inode);

    val = gpio_get_value(gpio_devs[minor].no);

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

ssize_t gpio_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *f_pos){

    char write_buf[count];
    int write_val;

    unsigned long err = copy_from_user(write_buf, ubuf, count);
    if(err < 0){
        printk("Error copying from user\n");
    }

    sscanf(write_buf, "%d", &write_val);

    int minor = iminor(filep->f_inode);

    gpio_set_value(gpio_devs[minor].no, write_val);

    return count;
}

static int gpio_pdrv_probe(struct platform_device *pdev){

    int err = 0;

    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    enum of_gpio_flags flag;
    int gpios_in_dt = 0;

    /* Retrieve number of GPIOs */
    if ((gpios_in_dt = of_gpio_count(np)) < 0){
        printk("Failed to read of_gpio_count\n");
        return -EINVAL;
    }

    printk("Number of GPIOs in DT: %d\n", gpios_in_dt);

    /* Loop through gpios in Device Tree */
    for(int i = 0; i < gpios_in_dt; i++){
        gpio_devs[i].no = of_get_gpio(np, i);
        if(gpio_devs[i].no < 0){
            if(gpio_devs[i].no != -EPROBE_DEFER){
                printk("Failed to parse io %d\n", gpio_devs[i].no);
            }
        }

        of_get_gpio_flags(np, i, &flag);
        gpio_devs[i].dir = flag;
        if(gpio_devs[i].dir < 0){
            if(gpio_devs[i].dir != -EPROBE_DEFER){
                printk("Failed to get flag for %d\n", gpio_devs[i].no);
            }
        }
    }
    /* Request gpio, set direction and create device for gpio */
    for(int i = 0; i < gpios_in_dt; i++){

        err = gpio_request(gpio_devs[i].no, NULL);
        if(err < 0){
            goto err_exit;
        }

        switch(gpio_devs[i].dir){
            case 0:
                gpio_direction_input(gpio_devs[i].no);
                break;
            case 1:
                gpio_direction_output(gpio_devs[i].no, 0);
                break;
            default:
                break;
        }

        gpio_device_101 = device_create(gpio_class, NULL, MKDEV(MAJOR(devno), i),
            NULL, "gpio%d", (101 + i));

        printk("GPIO with nr %d added with dir %d\n", gpio_devs[i].no, gpio_devs[i].dir);
    }

    printk("New GPIO platform device: %s\n", pdev->name);
    return 0;

    err_exit:
        return err;
}

static int gpio_pdrv_remove(struct platform_device *pdev){

    for(int i = 0; i < gpios_len; i++){
        device_destroy(gpio_class, MKDEV(MAJOR(devno), i));
        gpio_free(gpio_devs[i].no); //release claimed GPIO
    }

    printk("Removing GPIO device %s\n", pdev->name);
    return 0;
}

//Implemented file operations methods
struct file_operations gpio_fops = {
    .owner      = THIS_MODULE,
    .open       = gpio_open,
    .release    = gpio_release,
    .read       = gpio_read,
    .write      = gpio_write
};

static const struct of_device_id of_gpio_platform_device_match[] = {
    { .compatible = "ase, plat_drv",}, {},
};

static struct platform_driver gpio_platform_driver = {
    .probe  = gpio_pdrv_probe,
    .remove = gpio_pdrv_remove,
    .driver = {
        .name = "plat_drv",
        .of_match_table = of_gpio_platform_device_match,
        .owner = THIS_MODULE,
    },
};

module_init(gpio_init);
module_exit(gpio_exit);
