#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>

/* Interrupt header files */
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define SW_MAJOR 24
#define SW_MINOR 0
#define SW_MINOR_AMOUNT 1
#define SW_GPIO 16

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rene Street");
MODULE_DESCRIPTION("SW device driver for fHAT");

static int devno;

static struct cdev sw_cdev;
struct file_operations sw_fops;

static irqreturn_t sw_gpio_isr(int irq, void *dev_id);
static unsigned int sw_gpio_irq;

static DECLARE_WAIT_QUEUE_HEAD(read_wait);
static int read_flag = 0;

static int __init sw_init(void){
    //variable til fejl behandling
    int err = 0;

    //request GPIO 16. with NULL label
    err = gpio_request(SW_GPIO, NULL);
    if(err < 0 ){
        goto err_exit;
    }

    //Set GPIO port as input
    gpio_direction_input(SW_GPIO);
    if(err < 0 ){
        goto gpio_err;
    }

    //statisk allokering af major/minor number
    //create nodes med mknod
    devno = MKDEV(SW_MAJOR, SW_MINOR);

    //Register device
    err = register_chrdev_region(devno, SW_MINOR_AMOUNT, "sw1");
    if(err < 0){
        goto gpio_err;
    }

    //Tell kernel about the cdev structure - Final step
    cdev_init(&sw_cdev, &sw_fops);
    err = cdev_add(&sw_cdev, devno, 1);
    if(err < 0){
        goto err_dev_unregister;
    }

    sw_gpio_irq = gpio_to_irq(SW_GPIO);

    err = request_irq(sw_gpio_irq, &sw_gpio_isr, IRQF_TRIGGER_FALLING, "sw_gpio_irq", NULL);
    if(err){
        printk("sw_gpio: can't get assigned irq %d\n", sw_gpio_irq);
    } else {
        printk("sw_gpio: got assigned irq nr. %d\n", sw_gpio_irq);
    }

    return 0; //Success

    err_dev_unregister:
        unregister_chrdev_region(devno, SW_MINOR_AMOUNT); //unregister devices if error
    gpio_err:
        gpio_free(SW_GPIO); //release claimed GPIO
    err_exit:
        return err;
}

static void __exit sw_exit(void){
    free_irq(sw_gpio_irq, NULL);

    cdev_del(&sw_cdev); //Delete added cdev

    unregister_chrdev_region(devno, SW_MINOR_AMOUNT); //unregister device

    gpio_free(SW_GPIO); //release claimed GPIO
}

int swgpio_open(struct inode *inode, struct file *filep){
    int major, minor;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Opening SWGpio Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

int swgpio_release(struct inode *inode, struct file *filep){
    int minor, major;

    major = MAJOR(inode->i_rdev);
    minor = MINOR(inode->i_rdev);
    printk("Closing/Releasing SWGpio Device [major], [minor]: %i, %i\n", major, minor);

    return 0;
}

ssize_t swgpio_read(struct file *filep, char __user *buf,
    size_t count, loff_t *f_pos){

    wait_event_interruptible(read_wait, read_flag != 0);
    read_flag = 0;

    int val;
    char valbuf[16];

    val = gpio_get_value(SW_GPIO);
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

//Point to implemented file operations methods
struct file_operations sw_fops = {
    .owner      = THIS_MODULE,
    .open       = swgpio_open,
    .release    = swgpio_release,
    .read       = swgpio_read,
};

static irqreturn_t sw_gpio_isr(int irq, void *dev_id){
    //printk("IRQ event at irq line: %i\n", sw_gpio_irq); //debugging purposes

    read_flag = 1;
    wake_up_interruptible(&read_wait);

    return IRQ_HANDLED;
}


module_init(sw_init);
module_exit(sw_exit);
