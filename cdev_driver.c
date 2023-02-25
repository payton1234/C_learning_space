#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>

//for minor devices
#define MAX_DEV 2


// initialize file_operations
static const struct file_operations mychardev_fops = 
{
    .owner          = THIS_MODULE,
    .open           = mychardev_open,
    .release        = mychardev_release,
    .unlocked_ioctl = mychardev_ioctl,
    .read           = mychardev_read,
    .write          = mychardev_write
};

//device data holder
struct mychar_device_data
{
    struct cdev cdev;
}

//strorage for major number
static int dev_major = 0;

//sysfs class structure
static struct class * mychardev_class = NULL;

//array of mychar_device_data of
static struct mychar_device_data mychardev_data[MAX_DEV];

void mychardev_init(void)
{
    int err, i;
    dev_t dev;

    //allocate chardev and assign major number
    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "mychardev");

    dev_major = MAJOR(dev);

    //create necessary number of the devices
    for (i = 0, i < MAX_DEV; i++)
    {
        //init new device
        cdev_init(&mychardev_data[i].cdev, &mychardev_fops);
        mychardev_data[i].cdev.owner = THIS_MODULE;

        //add device to the system where "i" is a Minor number of the new device
        cdev_add(&mychardev_data[i].cdev, MKDEV(dev_major, i), 1);

        //creat device node /dev/mychardev-x where "x" is "i", equal to the minor number
        device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
    }
}


